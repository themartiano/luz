#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Mesh.hpp"
#include "OBJReader.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Lambertian.hpp"
#include <algorithm>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <filesystem>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <memory>
#include <thread>
#include <utility>
#include <cstdio>

namespace SceneFile::internal
{
	class	MeshLoadScheduler
	{
		public:
			explicit MeshLoadScheduler(std::size_t workerCount)
			{
				workerCount = std::max<std::size_t>(1, workerCount);
				this->_workers.reserve(workerCount);
				for (std::size_t i = 0; i < workerCount; i++)
				{
					this->_workers.emplace_back([this] { this->workerLoop(); });
				}
			}

			~MeshLoadScheduler(void)
			{
				{
					std::lock_guard<std::mutex> lock(this->_mutex);
					this->_stopping = true;
				}
				this->_condition.notify_all();
				for (std::thread& worker : this->_workers)
				{
					if (worker.joinable())
					{
						worker.join();
					}
				}
			}

			std::shared_future<std::shared_ptr<Hittable>>	enqueue(std::function<std::shared_ptr<Hittable>()> task)
			{
				auto promise = std::make_shared<std::promise<std::shared_ptr<Hittable>>>();
				std::shared_future<std::shared_ptr<Hittable>> future = promise->get_future().share();

				{
					std::lock_guard<std::mutex> lock(this->_mutex);
					if (this->_stopping)
					{
						throw std::runtime_error("Cannot enqueue mesh load after scheduler shutdown.");
					}
					this->_tasks.push([task = std::move(task), promise] {
						try
						{
							promise->set_value(task());
						}
						catch (...)
						{
							promise->set_exception(std::current_exception());
						}
					});
				}
				this->_condition.notify_one();

				return (future);
			}

		private:
			void	workerLoop(void)
			{
				while (true)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->_mutex);
						this->_condition.wait(lock, [this] {
							return (this->_stopping || !this->_tasks.empty());
						});
						if (this->_stopping && this->_tasks.empty())
						{
							return;
						}
						task = std::move(this->_tasks.front());
						this->_tasks.pop();
					}
					task();
				}
			}

			std::mutex	_mutex;
			std::condition_variable	_condition;
			std::queue<std::function<void()>>	_tasks;
			std::vector<std::thread>	_workers;
			bool	_stopping = false;
	};
}

namespace
{
	bool	splitAssignment(const std::string& line, std::string& key, std::string& value)
	{
		const std::size_t separator = line.find('=');

		if (separator == std::string::npos || separator == 0 || separator == line.length() - 1)
		{
			return (false);
		}
		key = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(line.substr(0, separator)));
		value = SceneFile::internal::_trim(line.substr(separator + 1));

		return (true);
	}

	std::shared_ptr<Material>	findNamedMaterial(const SceneFile::internal::SceneFileContext& context, const std::string& materialName)
	{
		const auto materialIt = context.materials.find(materialName);

		if (materialIt == context.materials.end())
		{
			throw std::runtime_error("Unknown material name: " + materialName);
		}

		return (materialIt->second);
	}

	std::string	findNamedMeshPath(const SceneFile::internal::SceneFileContext& context, const std::string& meshName)
	{
		const auto meshIt = context.meshes.find(meshName);

		if (meshIt == context.meshes.end())
		{
			throw std::runtime_error("Unknown mesh name: " + meshName);
		}

		return (meshIt->second);
	}

	std::pair<double, double>	parseVector2Value(const std::string& value, const std::string& label)
	{
		double x;
		double y;

		if (sscanf(SceneFile::internal::_trim(value).c_str(), "(%lf,%lf)", &x, &y) != 2)
		{
			throw std::runtime_error("Invalid " + label + " value. Use " + label + "=(x,y).");
		}

		return (std::make_pair(x, y));
	}

	ObjReadOptions	sceneObjReadOptions(const SceneFile::internal::SceneFileContext& context)
	{
		ObjReadOptions options;

		options.progress = context.meshLoadProgress;
		options.quiet = context.meshLoadProgress != nullptr;
		return (options);
	}

	class	DeferredHittable : public Hittable
	{
		public:
			explicit DeferredHittable(std::shared_future<std::shared_ptr<Hittable>> future)
				: _future(std::move(future))
			{
			}

			bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override
			{
				return (this->hittable()->hit(ray, hitRecord, t_min, t_max));
			}

			bool	hitAny(Ray& ray, double t_min, double t_max) const override
			{
				return (this->hittable()->hitAny(ray, t_min, t_max));
			}

			bool	createBoundingBox(AABB& outputBoundingBox) const override
			{
				return (this->hittable()->createBoundingBox(outputBoundingBox));
			}

			std::shared_ptr<Material>	getMaterial(void) const override
			{
				return (this->hittable()->getMaterial());
			}

			double	pdfValue(const Vector3& origin, const Vector3& vec) const override
			{
				return (this->hittable()->pdfValue(origin, vec));
			}

			Vector3	random(const Vector3& origin) const override
			{
				return (this->hittable()->random(origin));
			}

			bool	sampleLight(const Vector3& origin, HittableLightSample& sample) const override
			{
				return (this->hittable()->sampleLight(origin, sample));
			}

			double	lightSelectionWeight(void) const override
			{
				return (this->hittable()->lightSelectionWeight());
			}

		private:
			const std::shared_ptr<Hittable>&	hittable(void) const
			{
				std::call_once(this->_loadOnce, [this] {
					this->_hittable = this->_future.get();
				});
				return (this->_hittable);
			}

			std::shared_future<std::shared_ptr<Hittable>>	_future;
			mutable std::once_flag	_loadOnce;
			mutable std::shared_ptr<Hittable>	_hittable;
	};

	std::shared_ptr<Hittable>	scheduleMeshLoad(
		SceneFile::internal::SceneFileContext& context,
		const std::string& fileName,
		Vector3 position,
		Vector3 rotation,
		Vector3 scale,
		std::shared_ptr<Material> material
	)
	{
		const ObjReadOptions options = sceneObjReadOptions(context);
		if (!context.meshLoadScheduler)
		{
			context.meshLoadScheduler = std::make_shared<SceneFile::internal::MeshLoadScheduler>(context.meshLoadConcurrency);
		}

		std::shared_future<std::shared_ptr<Hittable>> future = context.meshLoadScheduler->enqueue(
			[fileName, position, rotation, scale, material, options] {
				return (std::make_shared<Mesh>(readObj(fileName, position, rotation, scale, material, options)));
			}
		);

		context.pendingMeshLoads.push_back(future);
		return (std::make_shared<DeferredHittable>(future));
	}

	struct ObjectBlock
	{
		std::string	meshName;
		std::string	fileName;
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		Vector3	rotation = Vector3(0.0, 0.0, 0.0);
		Vector3	scale = Vector3(1.0, 1.0, 1.0);
		std::shared_ptr<Material>	material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	};

	struct AreaLightBlock
	{
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		Vector3	normal = Vector3(0.0, -1.0, 0.0);
		double	width = 1.0;
		double	height = 1.0;
		Color	color = Color(1.0, 1.0, 1.0);
		double	intensity = 1.0;
	};

	struct SphereLightBlock
	{
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		double	radius = 0.1;
		Color	color = Color(1.0, 1.0, 1.0);
		double	intensity = 1.0;
	};

	void	addObjectBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& objectName)
	{
		std::string line;
		ObjectBlock object;

		do
		{
			getline(stream, line);
			const std::string blockLine = SceneFile::internal::_trim(line);

			if (blockLine.empty() || blockLine.at(0) == '#')
			{
				continue;
			}
			if (blockLine == "}")
			{
				if (!object.meshName.empty() && !object.fileName.empty())
				{
					throw std::runtime_error("Object block '" + objectName + "' defines both mesh and file.");
				}
				if (object.meshName.empty() && object.fileName.empty())
				{
					throw std::runtime_error("Object block '" + objectName + "' ended before mesh or file was defined.");
				}

				const std::string objectFileName = !object.fileName.empty()
					? SceneFile::internal::_resolveAssetPath(context.baseDirectory, object.fileName)
					: findNamedMeshPath(context, object.meshName);

				scene.addHittable(scheduleMeshLoad(
					context,
					objectFileName,
					object.position,
					object.rotation,
					object.scale,
					object.material
				));
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid object property: " + blockLine);
			}

			if (key == "mesh")
			{
				object.meshName = value;
			}
			else if (key == "file" || key == "path")
			{
				object.fileName = value;
			}
			else if (key == "position")
			{
				object.position = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "rotation")
			{
				object.rotation = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "scale")
			{
				object.scale = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "material")
			{
				object.material = findNamedMaterial(context, value);
			}
			else
			{
				throw std::runtime_error("Unknown object property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Object block '" + objectName + "' is missing a closing }.");
	}

	void	addAreaLightBlock(Scene& scene, std::ifstream& stream, const std::string& lightName)
	{
		std::string line;
		AreaLightBlock light;

		do
		{
			getline(stream, line);
			const std::string blockLine = SceneFile::internal::_trim(line);

			if (blockLine.empty() || blockLine.at(0) == '#')
			{
				continue;
			}
			if (blockLine == "}")
			{
				if (light.width <= 0.0 || light.height <= 0.0)
				{
					throw std::runtime_error("Area light '" + lightName + "' size must be positive.");
				}
				if (Utilities::vectorLengthSquared(light.normal) <= 0.0)
				{
					throw std::runtime_error("Area light '" + lightName + "' normal must be non-zero.");
				}

				scene.addHittable(std::make_shared<Rectangle>(
					Transform(light.position, light.normal, Vector3(1.0, 1.0, 1.0)),
					light.width,
					light.height,
					std::make_shared<Emissive>(light.color, light.intensity)
				));
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid area light property: " + blockLine);
			}

			if (key == "position")
			{
				light.position = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "normal" || key == "direction")
			{
				light.normal = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "size")
			{
				const std::pair<double, double> size = parseVector2Value(value, key);
				light.width = size.first;
				light.height = size.second;
			}
			else if (key == "width")
			{
				light.width = std::stod(value);
			}
			else if (key == "height")
			{
				light.height = std::stod(value);
			}
			else if (key == "color")
			{
				light.color = SceneFile::internal::_parseColorValue(value, key);
			}
			else if (key == "intensity")
			{
				light.intensity = std::stod(value);
			}
			else
			{
				throw std::runtime_error("Unknown area light property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Area light '" + lightName + "' is missing a closing }.");
	}

	void	addSphereLightBlock(Scene& scene, std::ifstream& stream, const std::string& lightName)
	{
		std::string line;
		SphereLightBlock light;

		do
		{
			getline(stream, line);
			const std::string blockLine = SceneFile::internal::_trim(line);

			if (blockLine.empty() || blockLine.at(0) == '#')
			{
				continue;
			}
			if (blockLine == "}")
			{
				if (light.radius <= 0.0)
				{
					throw std::runtime_error("Sphere light '" + lightName + "' radius must be positive.");
				}

				scene.addHittable(std::make_shared<Sphere>(
					light.position,
					light.radius,
					std::make_shared<Emissive>(light.color, light.intensity)
				));
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid sphere light property: " + blockLine);
			}

			if (key == "position")
			{
				light.position = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "radius")
			{
				light.radius = std::stod(value);
			}
			else if (key == "color")
			{
				light.color = SceneFile::internal::_parseColorValue(value, key);
			}
			else if (key == "intensity")
			{
				light.intensity = std::stod(value);
			}
			else
			{
				throw std::runtime_error("Unknown sphere light property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Sphere light '" + lightName + "' is missing a closing }.");
	}

	bool	addSceneObjectOrLightBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& line)
	{
		std::string blockName;

		if (SceneFile::internal::_parseNamedBlockHeader(line, "object", blockName))
		{
			addObjectBlock(scene, stream, context, blockName);
			return (true);
		}
		if (SceneFile::internal::_parseNamedBlockHeader(line, "area_light", blockName))
		{
			addAreaLightBlock(scene, stream, blockName);
			return (true);
		}
		if (
			SceneFile::internal::_parseNamedBlockHeader(line, "sphere_light", blockName)
			|| SceneFile::internal::_parseNamedBlockHeader(line, "point_light", blockName)
		)
		{
			addSphereLightBlock(scene, stream, blockName);
			return (true);
		}

		const std::string lowerLine = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(line));
		if (
			lowerLine.rfind("object ", 0) == 0
			|| lowerLine.rfind("area_light ", 0) == 0
			|| lowerLine.rfind("sphere_light ", 0) == 0
			|| lowerLine.rfind("point_light ", 0) == 0
		)
		{
			throw std::runtime_error("Invalid scene block header: " + line);
		}

		return (false);
	}
}

bool	SceneFile::internal::_readSceneObjectOrLightBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& line)
{
	return (addSceneObjectOrLightBlock(scene, stream, context, line));
}

// Parses the 'objects' sub-section of a Scene file
void	SceneFile::internal::_readObjectsSubSection(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context)
{
	std::string line;
	bool closed = false;

	do
	{
		getline(stream, line);

		if (line.length() <= 0 || line.at(0) == '#')
		{
			continue;
		}
		if (line == "}")
		{
			closed = true;
			break;
		}
		if (addSceneObjectOrLightBlock(scene, stream, context, line))
		{
			continue;
		}
		std::string lowerLine = line;
		Utilities::toLower(lowerLine);

		if (lowerLine.rfind("sphere=", 0) != std::string::npos)
		{
			double pX, pY, pZ, radius;

			if (sscanf(lowerLine.c_str(), "sphere=(%lf,%lf,%lf),%lf,material[", &pX, &pY, &pZ, &radius) == 4)
			{
				Sphere sphere;

				sphere.setPosition(Vector3(pX, pY, pZ));
				sphere.setRadius(radius);
				sphere.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Sphere>(sphere));

				continue;
			}
			throw std::runtime_error("Invalid sphere object: " + line);
		}
		else if (lowerLine.rfind("cube=", 0) != std::string::npos)
		{
			double pX, pY, pZ, oX, oY, oZ, width, height, depth;

			if (sscanf(lowerLine.c_str(), "cube=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,%lf,material[", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height, &depth) == 9)
			{
				Cube cube;

				cube.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
				cube.setWidth(width);
				cube.setHeight(height);
				cube.setDepth(depth);
				cube.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Cube>(cube));

				continue;
			}
			throw std::runtime_error("Invalid cube object: " + line);
		}
		else if (lowerLine.rfind("plane=", 0) != std::string::npos)
		{
			double y, oX, oY, oZ;

			if (sscanf(lowerLine.c_str(), "plane=%lf,(%lf,%lf,%lf),material[", &y, &oX, &oY, &oZ) == 4)
			{
				Plane plane;

				plane.setY(y);
				plane.setOrientation(Vector3(oX, oY, oZ));
				plane.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Plane>(plane));

				continue;
			}
			throw std::runtime_error("Invalid plane object: " + line);
		}
		else if (lowerLine.rfind("rectangle=", 0) != std::string::npos)
		{
			double pX, pY, pZ, oX, oY, oZ, width, height;

			if (sscanf(lowerLine.c_str(), "rectangle=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,material[", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height) == 8)
			{
				Rectangle rectangle;

				rectangle.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
				rectangle.setWidth(width);
				rectangle.setHeight(height);
				rectangle.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Rectangle>(rectangle));

				continue;
			}
			throw std::runtime_error("Invalid rectangle object: " + line);
		}
		else if (lowerLine.rfind("triangle=", 0) != std::string::npos)
		{
			double v0X, v0Y, v0Z, v1X, v1Y, v1Z, v2X, v2Y, v2Z;

			if (sscanf(lowerLine.c_str(), "triangle=(%lf,%lf,%lf),(%lf,%lf,%lf),(%lf,%lf,%lf),material[", &v0X, &v0Y, &v0Z, &v1X, &v1Y, &v1Z, &v2X, &v2Y, &v2Z) == 9)
			{
				Triangle triangle;

				triangle.setVertex0(Vector3(v0X, v0Y, v0Z));
				triangle.setVertex1(Vector3(v1X, v1Y, v1Z));
				triangle.setVertex2(Vector3(v2X, v2Y, v2Z));
				triangle.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Triangle>(triangle));

				continue;
			}
			throw std::runtime_error("Invalid triangle object: " + line);
		}
		else if (lowerLine.rfind("obj=", 0) != std::string::npos)
		{
			std::string strObjFileName = line.substr(std::string("obj=").size());
			char objFileName[1024];
			double pX, pY, pZ;

			if (sscanf(strObjFileName.c_str(), "%1023[^,],(%lf,%lf,%lf),material[", objFileName, &pX, &pY, &pZ) == 4)
			{
				scene.addHittable(scheduleMeshLoad(
					context,
					_resolveAssetPath(context.baseDirectory, objFileName),
					Vector3(pX, pY, pZ),
					Vector3(0.0, 0.0, 0.0),
					Vector3(1.0, 1.0, 1.0),
					internal::_readMaterialSubSection(stream)
				));

				continue;
			}

			if (!strObjFileName.empty())
			{
				scene.addHittable(scheduleMeshLoad(
					context,
					_resolveAssetPath(context.baseDirectory, strObjFileName),
					Vector3(0.0, 0.0, 0.0),
					Vector3(0.0, 0.0, 0.0),
					Vector3(1.0, 1.0, 1.0),
					std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))
				));

				continue;
			}
			throw std::runtime_error("Invalid OBJ object: " + line);
		}
		else
		{
			throw std::runtime_error("Unknown object line: " + line);
		}
	} while (!stream.eof());

	if (!closed)
	{
		throw std::runtime_error("Objects section is missing a closing }.");
	}
}
