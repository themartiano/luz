#include "SceneFile/SceneFile.hpp"
#include "Utilities.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Mesh.hpp"
#include "OBJReader.hpp"
#include <fstream>

// Parses the 'objects' sub-section of a Scene file
void	SceneFile::internal::_readObjectsSubSection(Scene& scene, std::ifstream& stream)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0 || line.at(0) == '#')
		{
			continue;
		}
		if (line == "}")
		{
			break;
		}
		Utilities::toLower(line);

		if (line.rfind("sphere=", 0) != std::string::npos)
		{
			double pX, pY, pZ, radius;

			if (sscanf(line.c_str(), "sphere=(%lf,%lf,%lf),%lf,material[\n", &pX, &pY, &pZ, &radius) != EOF)
			{
				Sphere sphere;

				sphere.setPosition(Vector3(pX, pY, pZ));
				sphere.setRadius(radius);
				sphere.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Sphere>(sphere));

				continue;
			}
		}
		else if (line.rfind("cube=", 0) != std::string::npos)
		{
			double pX, pY, pZ, oX, oY, oZ, width, height, depth;

			if (sscanf(line.c_str(), "cube=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,%lf,material[\n", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height, &depth) != EOF)
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
		}
		else if (line.rfind("plane=", 0) != std::string::npos)
		{
			double y, oX, oY, oZ;

			if (sscanf(line.c_str(), "plane=%lf,(%lf,%lf,%lf,material[\n", &y, &oX, &oY, &oZ) != EOF)
			{
				Plane plane;

				plane.setY(y);
				plane.setOrientation(Vector3(oX, oY, oZ));
				plane.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Plane>(plane));

				continue;
			}
		}
		else if (line.rfind("rectangle=", 0) != std::string::npos)
		{
			double pX, pY, pZ, oX, oY, oZ, width, height;

			if (sscanf(line.c_str(), "rectangle=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,material[\n", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height) != EOF)
			{
				Rectangle rectangle;

				rectangle.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
				rectangle.setWidth(width);
				rectangle.setHeight(height);
				rectangle.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Rectangle>(rectangle));

				continue;
			}
		}
		else if (line.rfind("triangle=", 0) != std::string::npos)
		{
			double v0X, v0Y, v0Z, v1X, v1Y, v1Z, v2X, v2Y, v2Z;

			if (sscanf(line.c_str(), "triangle=(%lf,%lf,%lf),(%lf,%lf,%lf),(%lf,%lf,%lf,material[\n", &v0X, &v0Y, &v0Z, &v1X, &v1Y, &v1Z, &v2X, &v2Y, &v2Z) != EOF)
			{
				Triangle triangle;

				triangle.setVertex0(Vector3(v0X, v0Y, v0Z));
				triangle.setVertex1(Vector3(v1X, v1Y, v1Z));
				triangle.setVertex2(Vector3(v2X, v2Y, v2Z));
				triangle.setMaterial(internal::_readMaterialSubSection(stream));

				scene.addHittable(std::make_shared<Triangle>(triangle));

				continue;
			}
		}
		else if (line.rfind("obj=", 0) != std::string::npos)
		{
			char objFileName[256];

			if (sscanf(line.c_str(), "obj=%s\n", objFileName) != EOF)
			{
				std::string strObjFileName(objFileName);

				scene.addHittable(std::make_shared<Mesh>(readObj(strObjFileName)));

				continue;
			}
		}
	} while (!stream.eof());
}
