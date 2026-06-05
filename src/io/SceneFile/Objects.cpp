#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Mesh.hpp"
#include "OBJReader.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>

static std::string	resolveAssetPath(const std::filesystem::path& baseDirectory, const std::string& assetPath)
{
	const std::filesystem::path path(assetPath);

	if (path.is_absolute())
	{
		return (path.string());
	}

	const std::filesystem::path sceneRelativePath = baseDirectory / path;
	if (!baseDirectory.empty() && std::filesystem::exists(sceneRelativePath))
	{
		return (sceneRelativePath.string());
	}

	if (std::filesystem::exists(path))
	{
		return (path.string());
	}

	const std::filesystem::path assetsPath = std::filesystem::path("assets/objects") / path;
	if (std::filesystem::exists(assetsPath))
	{
		return (assetsPath.string());
	}

	const std::filesystem::path objectsPath = std::filesystem::path("objects") / path;
	if (std::filesystem::exists(objectsPath))
	{
		return (objectsPath.string());
	}

	return (sceneRelativePath.string());
}

// Parses the 'objects' sub-section of a Scene file
void	SceneFile::internal::_readObjectsSubSection(Scene& scene, std::ifstream& stream, const std::filesystem::path& baseDirectory)
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
				scene.addHittable(std::make_shared<Mesh>(readObj(
					resolveAssetPath(baseDirectory, objFileName),
					Vector3(pX, pY, pZ),
					internal::_readMaterialSubSection(stream)
				)));

				continue;
			}

			if (!strObjFileName.empty())
			{
				scene.addHittable(std::make_shared<Mesh>(readObj(resolveAssetPath(baseDirectory, strObjFileName))));

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
