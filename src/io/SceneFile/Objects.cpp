#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/DirectionalLight.hpp"
#include "Hittables/ConstantVolume.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Mesh.hpp"
#include "OBJReader.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Isotropic.hpp"
#include "Materials/HenyeyGreenstein.hpp"
#include "ColorScience.hpp"
#include "LightUnits.hpp"
#include "MeasuredMaterials.hpp"
#include "Defaults.hpp"
#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <filesystem>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
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

	std::shared_ptr<Material>	readPrimitiveMaterial(
		const std::string& line,
		std::ifstream& stream,
		SceneFile::internal::SceneFileContext& context,
		int materialTokenEnd,
		const std::string& description
	)
	{
		const std::string suffix = SceneFile::internal::_trim(line.substr(materialTokenEnd));

		if (suffix == "[")
		{
			return (SceneFile::internal::_readMaterialSubSection(stream));
		}
		if (!suffix.empty() && suffix.at(0) == '=')
		{
			const std::string materialName = SceneFile::internal::_trim(suffix.substr(1));

			if (materialName.empty() || materialName.find_first_of(" \t{}[]") != std::string::npos)
			{
				throw std::runtime_error("Invalid " + description + " material name: " + line);
			}
			return (findNamedMaterial(context, materialName));
		}

		throw std::runtime_error(
			"Invalid "
			+ description
			+ " material. Use material[ for an inline material block or material=NAME for a named material."
		);
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

			Material*	getMaterial(void) const override
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

	struct PhysicalLightUnits
	{
		std::optional<double>	radiance;
		std::optional<double>	luminance;
		std::optional<double>	radiantPower;
		std::optional<double>	luminousFlux;
		std::optional<double>	radiantIntensity;
		std::optional<double>	luminousIntensity;
		std::optional<double>	irradiance;
		std::optional<double>	illuminance;
		std::optional<double>	solarScale;
	};

	struct AreaLightBlock
	{
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		Vector3	normal = Vector3(0.0, -1.0, 0.0);
		double	width = 1.0;
		double	height = 1.0;
		Color	color = Color(1.0, 1.0, 1.0);
		PhysicalLightUnits	units;
	};

	struct SphereLightBlock
	{
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		double	radius = 0.1;
		Color	color = Color(1.0, 1.0, 1.0);
		bool	visible = true;
		PhysicalLightUnits	units;
		std::string	iesProfilePath;
		Vector3	iesDirection = Vector3(0.0, -1.0, 0.0);
		double	iesRotationDegrees = 0.0;
	};

	struct SphereBlock
	{
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		double	radius = 1.0;
		std::shared_ptr<Material>	material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
		bool	visible = true;
		SphereUVProjection	uvProjection = SphereUVProjection::LatLong;
	};

	struct DirectionalLightBlock
	{
		Vector3	direction = Vector3(0.0, -1.0, 0.0);
		Color	color = Color(1.0, 1.0, 1.0);
		bool	hasColor = false;
		PhysicalLightUnits	units;
	};

	struct VolumeBlock
	{
		std::string	shape = "box";
		Vector3	position = Vector3(0.0, 0.0, 0.0);
		Vector3	size = Vector3(1.0, 1.0, 1.0);
		double	radius = 1.0;
		double	density = 0.1;
		bool	hasDensity = false;
		Color	color = Color(0.72, 0.78, 0.86);
		bool	hasColor = false;
		double	anisotropy = 0.0;
		bool	hasAnisotropy = false;
		double	densityScale = 1.0;
		std::string	volumePreset;
		std::optional<Color>	scatteringCoefficient;
		std::optional<Color>	absorptionCoefficient;
		std::shared_ptr<Material>	material = nullptr;
	};

	void	requirePositiveFinite(double value, const std::string& description)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			throw std::runtime_error(description + " must be finite and positive.");
		}
	}

	bool	hasPhysicalLightUnit(const PhysicalLightUnits& units)
	{
		return (
			units.radiance
			|| units.luminance
			|| units.radiantPower
			|| units.luminousFlux
			|| units.radiantIntensity
			|| units.luminousIntensity
			|| units.irradiance
			|| units.illuminance
			|| units.solarScale
		);
	}

	void	assignPhysicalLightUnit(std::optional<double>& destination, PhysicalLightUnits& units, const std::string& value)
	{
		if (hasPhysicalLightUnit(units))
		{
			throw std::runtime_error("Light block defines multiple physical light quantities.");
		}
		destination = std::stod(value);
	}

	bool	parseSurfaceLightUnit(PhysicalLightUnits& units, const std::string& key, const std::string& value)
	{
		if (key == "radiance" || key == "surface_radiance" || key == "surfaceradiance")
		{
			assignPhysicalLightUnit(units.radiance, units, value);
			return (true);
		}
		if (key == "luminance" || key == "nits" || key == "cd_m2" || key == "cdm2")
		{
			assignPhysicalLightUnit(units.luminance, units, value);
			return (true);
		}
		if (key == "power" || key == "watts" || key == "radiant_power" || key == "radiantpower")
		{
			assignPhysicalLightUnit(units.radiantPower, units, value);
			return (true);
		}
		if (
			key == "lumens"
			|| key == "lumen"
			|| key == "lm"
			|| key == "luminous_flux"
			|| key == "luminousflux"
		)
		{
			assignPhysicalLightUnit(units.luminousFlux, units, value);
			return (true);
		}
		return (false);
	}

	bool	parseSphericalLightUnit(PhysicalLightUnits& units, const std::string& key, const std::string& value)
	{
		if (parseSurfaceLightUnit(units, key, value))
		{
			return (true);
		}
		if (
			key == "radiant_intensity"
			|| key == "radiantintensity"
			|| key == "w_sr"
			|| key == "wsr"
		)
		{
			assignPhysicalLightUnit(units.radiantIntensity, units, value);
			return (true);
		}
		if (
			key == "candela"
			|| key == "cd"
			|| key == "luminous_intensity"
			|| key == "luminousintensity"
		)
		{
			assignPhysicalLightUnit(units.luminousIntensity, units, value);
			return (true);
		}
		return (false);
	}

	bool	parseDirectionalLightUnit(PhysicalLightUnits& units, const std::string& key, const std::string& value)
	{
		if (key == "irradiance" || key == "w_m2" || key == "wm2")
		{
			assignPhysicalLightUnit(units.irradiance, units, value);
			return (true);
		}
		if (key == "illuminance" || key == "lux" || key == "lx")
		{
			assignPhysicalLightUnit(units.illuminance, units, value);
			return (true);
		}
		if (key == "solar" || key == "sun" || key == "solar_scale" || key == "solarscale")
		{
			assignPhysicalLightUnit(units.solarScale, units, value);
			return (true);
		}
		return (false);
	}

	Color	surfaceLightEmission(Color color, const PhysicalLightUnits& units, double area)
	{
		if (units.radiance)
		{
			return (LightUnits::surfaceRadiance(color, *units.radiance));
		}
		if (units.luminance)
		{
			return (LightUnits::surfaceLuminance(color, *units.luminance));
		}
		if (units.radiantPower)
		{
			return (LightUnits::surfaceRadiantPower(color, *units.radiantPower, area));
		}
		if (units.luminousFlux)
		{
			return (LightUnits::surfaceLuminousFlux(color, *units.luminousFlux, area));
		}
		if (units.radiantIntensity)
		{
			return (LightUnits::sphericalRadiantIntensity(color, *units.radiantIntensity, area));
		}
		if (units.luminousIntensity)
		{
			return (LightUnits::sphericalLuminousIntensity(color, *units.luminousIntensity, area));
		}
		throw std::runtime_error("Surface light must define radiance, luminance, power, or lumens.");
	}

	Color	sphericalLightEmission(Color color, const PhysicalLightUnits& units, double area, const std::shared_ptr<IESProfile>& iesProfile)
	{
		if (hasPhysicalLightUnit(units))
		{
			return (surfaceLightEmission(color, units, area));
		}
		if (iesProfile)
		{
			return (LightUnits::surfaceLuminousFlux(color, iesProfile->totalLumens(), area));
		}
		throw std::runtime_error("Sphere light must define radiance, luminance, power, lumens, candela, radiant_intensity, or ies.");
	}

	Color	directionalLightEmission(Color color, const PhysicalLightUnits& units)
	{
		if (units.irradiance)
		{
			return (LightUnits::directionalIrradiance(color, *units.irradiance));
		}
		if (units.illuminance)
		{
			return (LightUnits::directionalIlluminance(color, *units.illuminance));
		}
		if (units.solarScale)
		{
			return (LightUnits::solarDirectionalIrradiance(color, *units.solarScale));
		}
		throw std::runtime_error("Directional light must define irradiance, illuminance, or solar.");
	}

	SphereUVProjection	parseSphereUVProjection(const std::string& value)
	{
		const std::string projection = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(value));

		if (
			projection == "latlong"
			|| projection == "lat_long"
			|| projection == "longitude_latitude"
			|| projection == "equirectangular"
		)
		{
			return (SphereUVProjection::LatLong);
		}
		if (
			projection == "cube_cross"
			|| projection == "cubecross"
			|| projection == "cross"
			|| projection == "cube"
		)
		{
			return (SphereUVProjection::CubeCross);
		}

		throw std::runtime_error("Unknown sphere uv projection: " + value);
	}

	void	requireVolumeSize(const Vector3& size, const std::string& volumeName)
	{
		requirePositiveFinite(size.getX(), "Volume '" + volumeName + "' width");
		requirePositiveFinite(size.getY(), "Volume '" + volumeName + "' height");
		requirePositiveFinite(size.getZ(), "Volume '" + volumeName + "' depth");
	}

	std::shared_ptr<Material>	buildVolumePhaseFunction(const VolumeBlock& volume)
	{
		if (volume.material)
		{
			return (volume.material);
		}
		if (std::fabs(volume.anisotropy) <= 1e-12)
		{
			return (std::make_shared<Isotropic>(volume.color));
		}
		return (std::make_shared<HenyeyGreenstein>(volume.color, volume.anisotropy));
	}

	std::shared_ptr<Hittable>	buildVolumeBoundary(const VolumeBlock& volume, const std::string& volumeName)
	{
		const std::string shape = SceneFile::internal::_lowerCopy(volume.shape);
		const auto boundaryMaterial = std::make_shared<Lambertian>(Color(0.0, 0.0, 0.0));

		if (shape == "sphere")
		{
			requirePositiveFinite(volume.radius, "Volume '" + volumeName + "' radius");
			return (std::make_shared<Sphere>(volume.position, volume.radius, boundaryMaterial));
		}
		if (shape == "box" || shape == "cube")
		{
			requireVolumeSize(volume.size, volumeName);
			return (std::make_shared<Cube>(
				Transform(volume.position, Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
				volume.size.getX(),
				volume.size.getY(),
				volume.size.getZ(),
				boundaryMaterial
			));
		}

		throw std::runtime_error("Unknown volume shape: " + volume.shape);
	}

	double	sceneVolumeDensity(const Scene& scene, double density, const std::string& volumeName)
	{
		requirePositiveFinite(density, "Volume '" + volumeName + "' density");
		const double sceneDensity = density * scene.getMetersPerUnit();
		requirePositiveFinite(sceneDensity, "Volume '" + volumeName + "' scene-unit density");
		return (sceneDensity);
	}

	Color	scaledColor(Color color, double scale)
	{
		return (Color(
			color.getRed() * scale,
			color.getGreen() * scale,
			color.getBlue() * scale
		));
	}

	void	applyMeasuredVolumeCoefficients(
		VolumeBlock& volume,
		Color scatteringCoefficient,
		Color absorptionCoefficient,
		const std::string& volumeName
	)
	{
		if (!std::isfinite(volume.densityScale) || volume.densityScale <= 0.0)
		{
			throw std::runtime_error("Volume '" + volumeName + "' density_scale must be finite and positive.");
		}

		scatteringCoefficient = scaledColor(scatteringCoefficient, volume.densityScale);
		absorptionCoefficient = scaledColor(absorptionCoefficient, volume.densityScale);
		volume.density = MeasuredMaterials::volumeDensity(scatteringCoefficient, absorptionCoefficient);
		volume.color = MeasuredMaterials::volumeScatteringAlbedo(scatteringCoefficient, volume.density);
	}

	void	applyMeasuredVolumeData(VolumeBlock& volume, const std::string& volumeName)
	{
		if (!volume.volumePreset.empty())
		{
			if (volume.scatteringCoefficient || volume.absorptionCoefficient)
			{
				throw std::runtime_error("Volume '" + volumeName + "' defines both preset and explicit sigma coefficients.");
			}
			if (volume.hasDensity || volume.hasColor || volume.hasAnisotropy)
			{
				throw std::runtime_error("Volume '" + volumeName + "' preset cannot be combined with density, color, or anisotropy overrides. Use density_scale.");
			}
			const MeasuredMaterials::Volume preset = MeasuredMaterials::volumePreset(volume.volumePreset);
			volume.anisotropy = preset.anisotropy;
			applyMeasuredVolumeCoefficients(
				volume,
				preset.scatteringCoefficient,
				preset.absorptionCoefficient,
				volumeName
			);
			return;
		}
		if (volume.scatteringCoefficient || volume.absorptionCoefficient)
		{
			if (volume.hasDensity || volume.hasColor)
			{
				throw std::runtime_error("Volume '" + volumeName + "' sigma coefficients cannot be combined with density or color.");
			}
			applyMeasuredVolumeCoefficients(
				volume,
				volume.scatteringCoefficient.value_or(Color(0.0, 0.0, 0.0)),
				volume.absorptionCoefficient.value_or(Color(0.0, 0.0, 0.0)),
				volumeName
			);
		}
	}

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

	void	addSphereBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& sphereName)
	{
		std::string line;
		SphereBlock sphere;

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
				requirePositiveFinite(sphere.radius, "Sphere object '" + sphereName + "' radius");
				scene.addHittable(std::make_shared<Sphere>(
					sphere.position,
					sphere.radius,
					sphere.material,
					sphere.visible,
					sphere.uvProjection
				));
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid sphere property: " + blockLine);
			}

			if (key == "position" || key == "center")
			{
				sphere.position = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "radius")
			{
				sphere.radius = std::stod(value);
			}
			else if (key == "material")
			{
				sphere.material = findNamedMaterial(context, value);
			}
			else if (key == "visible")
			{
				int visible;

				if (sscanf(value.c_str(), "%d", &visible) != 1 || (visible != 0 && visible != 1))
				{
					throw std::runtime_error("Sphere object '" + sphereName + "' visible must be 0 or 1.");
				}
				sphere.visible = visible != 0;
			}
			else if (key == "uv_projection" || key == "uvprojection" || key == "texture_projection" || key == "projection")
			{
				sphere.uvProjection = parseSphereUVProjection(value);
			}
			else
			{
				throw std::runtime_error("Unknown sphere property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Sphere object '" + sphereName + "' is missing a closing }.");
	}

	void	addAreaLightBlock(
		Scene& scene,
		std::ifstream& stream,
		SceneFile::internal::SceneFileContext& context,
		const std::string& lightName
	)
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
					std::make_shared<Emissive>(
						surfaceLightEmission(
							light.color,
							light.units,
							scene.sceneAreaToSquareMeters(light.width * light.height)
						)
					)
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
				light.color = SceneFile::internal::_parseColorValue(value, key, context);
			}
			else if (parseSurfaceLightUnit(light.units, key, value))
			{
				continue;
			}
			else
			{
				throw std::runtime_error("Unknown area light property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Area light '" + lightName + "' is missing a closing }.");
	}

	void	addSphereLightBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& lightName)
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

				std::shared_ptr<IESProfile> iesProfile;
				if (!light.iesProfilePath.empty())
				{
					iesProfile = std::make_shared<IESProfile>(IESProfile::load(
						SceneFile::internal::_resolveAssetPath(context.baseDirectory, light.iesProfilePath)
					));
				}
				std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(
					light.position,
					light.radius,
					std::make_shared<Emissive>(
						sphericalLightEmission(
							light.color,
							light.units,
							scene.sceneAreaToSquareMeters(4.0 * D_PI * light.radius * light.radius),
							iesProfile
						)
					),
					light.visible
				);
				if (iesProfile)
				{
					sphere->setIESProfile(iesProfile, light.iesDirection, light.iesRotationDegrees);
				}
				scene.addHittable(sphere);
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
				light.color = SceneFile::internal::_parseColorValue(value, key, context);
			}
			else if (parseSphericalLightUnit(light.units, key, value))
			{
				continue;
			}
			else if (key == "ies" || key == "ies_profile" || key == "iesprofile" || key == "profile")
			{
				light.iesProfilePath = value;
			}
			else if (key == "ies_direction" || key == "iesdirection" || key == "profile_direction" || key == "profiledirection")
			{
				light.iesDirection = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "ies_rotation" || key == "iesrotation" || key == "profile_rotation" || key == "profilerotation")
			{
				light.iesRotationDegrees = std::stod(value);
			}
			else if (key == "visible")
			{
				int visible;

				if (sscanf(value.c_str(), "%d", &visible) != 1 || (visible != 0 && visible != 1))
				{
					throw std::runtime_error("Sphere light '" + lightName + "' visible must be 0 or 1.");
				}
				light.visible = visible != 0;
			}
			else
			{
				throw std::runtime_error("Unknown sphere light property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Sphere light '" + lightName + "' is missing a closing }.");
	}

	void	addDirectionalLightBlock(
		Scene& scene,
		std::ifstream& stream,
		SceneFile::internal::SceneFileContext& context,
		const std::string& lightName
	)
	{
		std::string line;
		DirectionalLightBlock light;

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
				if (Utilities::vectorLengthSquared(light.direction) <= 0.0)
				{
					throw std::runtime_error("Directional light '" + lightName + "' direction must be non-zero.");
				}

				const Color color = (light.units.solarScale && !light.hasColor)
					? ColorScience::solar()
					: light.color;
				std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>(
					light.direction,
					std::make_shared<Emissive>(
						directionalLightEmission(color, light.units)
					)
				);
				if (light.units.solarScale)
				{
					directionalLight->setAtmosphereSunRadiance(
						LightUnits::solarDirectionalIrradiance(color, *light.units.solarScale)
					);
				}
				scene.addHittable(directionalLight);
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid directional light property: " + blockLine);
			}

			if (key == "direction")
			{
				light.direction = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "color")
			{
				light.color = SceneFile::internal::_parseColorValue(value, key, context);
				light.hasColor = true;
			}
			else if (parseDirectionalLightUnit(light.units, key, value))
			{
				continue;
			}
			else
			{
				throw std::runtime_error("Unknown directional light property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Directional light '" + lightName + "' is missing a closing }.");
	}

	void	addVolumeBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& volumeName)
	{
		std::string line;
		VolumeBlock volume;

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
				applyMeasuredVolumeData(volume, volumeName);
				scene.addHittable(std::make_shared<ConstantVolume>(
					buildVolumeBoundary(volume, volumeName),
					buildVolumePhaseFunction(volume),
					sceneVolumeDensity(scene, volume.density, volumeName)
				));
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid volume property: " + blockLine);
			}

			if (key == "shape" || key == "type")
			{
				volume.shape = SceneFile::internal::_lowerCopy(value);
			}
			else if (key == "position" || key == "center")
			{
				volume.position = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "size" || key == "dimensions")
			{
				volume.size = SceneFile::internal::_parseVector3Value(value, key);
			}
			else if (key == "width")
			{
				volume.size.setX(std::stod(value));
			}
			else if (key == "height")
			{
				volume.size.setY(std::stod(value));
			}
			else if (key == "depth")
			{
				volume.size.setZ(std::stod(value));
			}
			else if (key == "radius")
			{
				volume.radius = std::stod(value);
			}
			else if (key == "density" || key == "extinction" || key == "sigma_t")
			{
				volume.density = std::stod(value);
				volume.hasDensity = true;
			}
			else if (key == "color" || key == "albedo" || key == "scatteringcolor" || key == "scattering_color")
			{
				volume.color = SceneFile::internal::_parseColorValue(value, key, context);
				volume.hasColor = true;
			}
			else if (key == "anisotropy" || key == "g")
			{
				volume.anisotropy = std::stod(value);
				volume.hasAnisotropy = true;
			}
			else if (key == "preset" || key == "volume_preset" || key == "volumepreset" || key == "medium")
			{
				volume.volumePreset = value;
			}
			else if (key == "density_scale" || key == "densityscale" || key == "coefficient_scale" || key == "coefficientscale")
			{
				volume.densityScale = std::stod(value);
			}
			else if (
				key == "sigma_s"
				|| key == "sigmas"
				|| key == "scattering"
				|| key == "scattering_coefficient"
				|| key == "scatteringcoefficient"
			)
			{
				volume.scatteringCoefficient = SceneFile::internal::_parseColorValue(value, key, context);
			}
			else if (
				key == "sigma_a"
				|| key == "sigmaa"
				|| key == "absorption"
				|| key == "absorption_coefficient"
				|| key == "absorptioncoefficient"
			)
			{
				volume.absorptionCoefficient = SceneFile::internal::_parseColorValue(value, key, context);
			}
			else if (key == "material")
			{
				volume.material = findNamedMaterial(context, value);
			}
			else
			{
				throw std::runtime_error("Unknown volume property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Volume '" + volumeName + "' is missing a closing }.");
	}

	bool	addSceneObjectOrLightBlock(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context, const std::string& line)
	{
		std::string blockName;

		if (SceneFile::internal::_parseNamedBlockHeader(line, "object", blockName))
		{
			addObjectBlock(scene, stream, context, blockName);
			return (true);
		}
		if (SceneFile::internal::_parseNamedBlockHeader(line, "sphere", blockName))
		{
			addSphereBlock(scene, stream, context, blockName);
			return (true);
		}
		if (SceneFile::internal::_parseNamedBlockHeader(line, "area_light", blockName))
		{
			addAreaLightBlock(scene, stream, context, blockName);
			return (true);
		}
		if (SceneFile::internal::_parseNamedBlockHeader(line, "directional_light", blockName))
		{
			addDirectionalLightBlock(scene, stream, context, blockName);
			return (true);
		}
		if (SceneFile::internal::_parseNamedBlockHeader(line, "volume", blockName))
		{
			addVolumeBlock(scene, stream, context, blockName);
			return (true);
		}
		if (
			SceneFile::internal::_parseNamedBlockHeader(line, "sphere_light", blockName)
			|| SceneFile::internal::_parseNamedBlockHeader(line, "point_light", blockName)
		)
		{
			addSphereLightBlock(scene, stream, context, blockName);
			return (true);
		}

		const std::string lowerLine = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(line));
		if (
			lowerLine.rfind("object ", 0) == 0
			|| lowerLine.rfind("sphere ", 0) == 0
			|| lowerLine.rfind("area_light ", 0) == 0
			|| lowerLine.rfind("directional_light ", 0) == 0
			|| lowerLine.rfind("volume ", 0) == 0
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
		const std::string trimmedLine = SceneFile::internal::_trim(line);

		if (trimmedLine.empty() || trimmedLine.at(0) == '#')
		{
			continue;
		}
		if (trimmedLine == "}")
		{
			closed = true;
			break;
		}
		if (addSceneObjectOrLightBlock(scene, stream, context, trimmedLine))
		{
			continue;
		}
		std::string lowerLine = SceneFile::internal::_lowerCopy(trimmedLine);

		if (lowerLine.rfind("sphere=", 0) != std::string::npos)
		{
			double pX, pY, pZ, radius;
			int materialPosition = 0;

			if (sscanf(lowerLine.c_str(), "sphere=(%lf,%lf,%lf),%lf,material%n", &pX, &pY, &pZ, &radius, &materialPosition) == 4)
			{
				Sphere sphere;

				sphere.setPosition(Vector3(pX, pY, pZ));
				sphere.setRadius(radius);
				sphere.setMaterial(readPrimitiveMaterial(
					trimmedLine,
					stream,
					context,
					materialPosition,
					"sphere object"
				));

				scene.addHittable(std::make_shared<Sphere>(sphere));

				continue;
			}
			throw std::runtime_error("Invalid sphere object: " + line);
		}
		else if (lowerLine.rfind("cube=", 0) != std::string::npos)
		{
			double pX, pY, pZ, oX, oY, oZ, width, height, depth;
			int materialPosition = 0;

			if (sscanf(lowerLine.c_str(), "cube=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,%lf,material%n", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height, &depth, &materialPosition) == 9)
			{
				Cube cube;

				cube.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
				cube.setWidth(width);
				cube.setHeight(height);
				cube.setDepth(depth);
				cube.setMaterial(readPrimitiveMaterial(
					trimmedLine,
					stream,
					context,
					materialPosition,
					"cube object"
				));

				scene.addHittable(std::make_shared<Cube>(cube));

				continue;
			}
			throw std::runtime_error("Invalid cube object: " + line);
		}
		else if (lowerLine.rfind("plane=", 0) != std::string::npos)
		{
			double y, oX, oY, oZ;
			int materialPosition = 0;

			if (sscanf(lowerLine.c_str(), "plane=%lf,(%lf,%lf,%lf),material%n", &y, &oX, &oY, &oZ, &materialPosition) == 4)
			{
				Plane plane;

				plane.setY(y);
				plane.setOrientation(Vector3(oX, oY, oZ));
				plane.setMaterial(readPrimitiveMaterial(
					trimmedLine,
					stream,
					context,
					materialPosition,
					"plane object"
				));

				scene.addHittable(std::make_shared<Plane>(plane));

				continue;
			}
			throw std::runtime_error("Invalid plane object: " + line);
		}
		else if (lowerLine.rfind("rectangle=", 0) != std::string::npos)
		{
			double pX, pY, pZ, oX, oY, oZ, width, height;
			int materialPosition = 0;

			if (sscanf(lowerLine.c_str(), "rectangle=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,material%n", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height, &materialPosition) == 8)
			{
				Rectangle rectangle;

				rectangle.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
				rectangle.setWidth(width);
				rectangle.setHeight(height);
				rectangle.setMaterial(readPrimitiveMaterial(
					trimmedLine,
					stream,
					context,
					materialPosition,
					"rectangle object"
				));

				scene.addHittable(std::make_shared<Rectangle>(rectangle));

				continue;
			}
			throw std::runtime_error("Invalid rectangle object: " + line);
		}
		else if (lowerLine.rfind("triangle=", 0) != std::string::npos)
		{
			double v0X, v0Y, v0Z, v1X, v1Y, v1Z, v2X, v2Y, v2Z;
			int materialPosition = 0;

			if (sscanf(lowerLine.c_str(), "triangle=(%lf,%lf,%lf),(%lf,%lf,%lf),(%lf,%lf,%lf),material%n", &v0X, &v0Y, &v0Z, &v1X, &v1Y, &v1Z, &v2X, &v2Y, &v2Z, &materialPosition) == 9)
			{
				Triangle triangle;

				triangle.setVertex0(Vector3(v0X, v0Y, v0Z));
				triangle.setVertex1(Vector3(v1X, v1Y, v1Z));
				triangle.setVertex2(Vector3(v2X, v2Y, v2Z));
				triangle.setMaterial(readPrimitiveMaterial(
					trimmedLine,
					stream,
					context,
					materialPosition,
					"triangle object"
				));

				scene.addHittable(std::make_shared<Triangle>(triangle));

				continue;
			}
			throw std::runtime_error("Invalid triangle object: " + line);
		}
		else if (lowerLine.rfind("obj=", 0) != std::string::npos)
		{
			std::string strObjFileName = trimmedLine.substr(std::string("obj=").size());
			char objFileName[1024];
			double pX, pY, pZ;
			int materialPosition = 0;

			if (sscanf(strObjFileName.c_str(), "%1023[^,],(%lf,%lf,%lf),material%n", objFileName, &pX, &pY, &pZ, &materialPosition) == 4)
			{
				scene.addHittable(scheduleMeshLoad(
					context,
					_resolveAssetPath(context.baseDirectory, objFileName),
					Vector3(pX, pY, pZ),
					Vector3(0.0, 0.0, 0.0),
					Vector3(1.0, 1.0, 1.0),
					readPrimitiveMaterial(
						strObjFileName,
						stream,
						context,
						materialPosition,
						"OBJ object"
					)
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
			throw std::runtime_error("Unknown object line: " + trimmedLine);
		}
	} while (!stream.eof());

	if (!closed)
	{
		throw std::runtime_error("Objects section is missing a closing }.");
	}
}
