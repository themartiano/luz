#include "OBJReader.hpp"
#include "Hittables/Hittable.hpp"
#include "Hittables/BVHNode.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Mesh.hpp"
#include "Vector3.hpp"
#include "Utilities.hpp"
#include "ANSIColors.hpp"
#include "Clock.hpp"
#include "Materials/Lambertian.hpp"
#include "Random.hpp"
#include <memory>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cmath>

static void	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset, Vector3 rotationDegrees, Vector3 scale, std::shared_ptr<Material> material);

namespace
{
	const double	DEGENERATE_TRIANGLE_EPSILON_SQUARED = 1e-24;
	const double	DEGREES_TO_RADIANS = 3.14159265358979323846 / 180.0;

	Vector3	rotateVertex(Vector3 vertex, Vector3 rotationDegrees)
	{
		const double xRadians = rotationDegrees.getX() * DEGREES_TO_RADIANS;
		const double yRadians = rotationDegrees.getY() * DEGREES_TO_RADIANS;
		const double zRadians = rotationDegrees.getZ() * DEGREES_TO_RADIANS;

		double x = vertex.getX();
		double y = vertex.getY();
		double z = vertex.getZ();

		const double cosX = std::cos(xRadians);
		const double sinX = std::sin(xRadians);
		double nextY = (y * cosX) - (z * sinX);
		double nextZ = (y * sinX) + (z * cosX);
		y = nextY;
		z = nextZ;

		const double cosY = std::cos(yRadians);
		const double sinY = std::sin(yRadians);
		double nextX = (x * cosY) + (z * sinY);
		nextZ = (-x * sinY) + (z * cosY);
		x = nextX;
		z = nextZ;

		const double cosZ = std::cos(zRadians);
		const double sinZ = std::sin(zRadians);
		nextX = (x * cosZ) - (y * sinZ);
		nextY = (x * sinZ) + (y * cosZ);
		x = nextX;
		y = nextY;

		return (Vector3(x, y, z));
	}

	Vector3	transformVertex(Vector3 vertex, Vector3 positionOffset, Vector3 rotationDegrees, Vector3 scale)
	{
		vertex = vertex * scale;
		vertex = rotateVertex(vertex, rotationDegrees);

		return (vertex + positionOffset);
	}
}

// Calls the actual 'readObj' function with a zeroed offset position
Mesh	readObj(std::string fileName)
{
	return (readObj(fileName, Vector3(0.0, 0.0, 0.0), std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))));
}

// Search and read / parse the obj file named 'fileName' (current directory)
Mesh	readObj(std::string fileName, Vector3 positionOffset, std::shared_ptr<Material> material)
{
	return (readObj(fileName, positionOffset, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), material));
}

Mesh	readObj(std::string fileName, Vector3 positionOffset, Vector3 rotationDegrees, Vector3 scale, std::shared_ptr<Material> material)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		throw std::runtime_error("OBJ file could not be opened: " + fileName);
	}

	std::cout << CLR_YELLOW << "Parsing " << CLR_BLUE << fileName << CLR_YELLOW << "..." << CLR_RESET << std::endl;

	Clock	clock;
	Mesh	mesh;

	clock.start();
	parseObjFile(mesh, stream, positionOffset, rotationDegrees, scale, material);

	std::cout << CLR_BLUE << fileName << CLR_GREEN_BRIGHT << " parsing done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << clock.elapsedS() << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;

	return (mesh);
}

static void	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset, Vector3 rotationDegrees, Vector3 scale, std::shared_ptr<Material> material)
{
	std::string line;
	std::vector<Vector3> vertices;
	std::vector<std::shared_ptr<Hittable>> triangles;
	std::size_t skippedDegenerateTriangles = 0;

	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			continue;
		}

		if (line.rfind("v ", 0) == 0)
		{
			Vector3	vertex;
			size_t oldPos = 2;
			size_t newPos = 0;

			newPos = line.find_first_of(' ', oldPos);
			vertex.setX(std::stod(line.substr(oldPos, newPos - oldPos)));
			oldPos = newPos + 1;

			newPos = line.find_first_of(' ', oldPos);
			vertex.setY(std::stod(line.substr(oldPos, newPos - oldPos)));
			oldPos = newPos + 1;

			newPos = line.find_first_of(' ', oldPos);
			vertex.setZ(std::stod(line.substr(oldPos, newPos - oldPos)));

			vertices.push_back(vertex);
		}
		else if (line.rfind("f ", 0) == 0)
		{
			std::vector<int> positions(3);
			size_t oldPos = 2;
			size_t newPos = 0;

			newPos = line.find_first_of(' ', oldPos);
			positions[0] = std::stoi(line.substr(oldPos, newPos - oldPos));
			oldPos = newPos + 1;

			newPos = line.find_first_of(' ', oldPos);
			positions[1] = std::stoi(line.substr(oldPos, newPos - oldPos));
			oldPos = newPos + 1;

			newPos = line.find_first_of(' ', oldPos);
			positions[2] = std::stoi(line.substr(oldPos, newPos - oldPos));

			Vector3 vertex0 = transformVertex(vertices[positions[0] - 1], positionOffset, rotationDegrees, scale);
			Vector3 vertex1 = transformVertex(vertices[positions[1] - 1], positionOffset, rotationDegrees, scale);
			Vector3 vertex2 = transformVertex(vertices[positions[2] - 1], positionOffset, rotationDegrees, scale);
			Vector3 normal = Utilities::cross(vertex1 - vertex0, vertex2 - vertex0);

			if (Utilities::vectorLengthSquared(normal) <= DEGENERATE_TRIANGLE_EPSILON_SQUARED)
			{
				skippedDegenerateTriangles++;
				continue;
			}

			triangles.push_back(std::make_shared<Triangle>(
				vertex0,
				vertex1,
				vertex2,
				//Material(Color(Random::doubleFloat(), Random::doubleFloat(), Random::doubleFloat()), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
				//std::make_shared<Dielectric>(Color(0.8, 0.8, 0.8))
				material
			));
		}
	} while (!stream.eof());

	if (skippedDegenerateTriangles > 0)
	{
		std::cout
			<< CLR_YELLOW << "Skipped " << CLR_WHITE << skippedDegenerateTriangles
			<< CLR_YELLOW << " degenerate OBJ triangle"
			<< (skippedDegenerateTriangles == 1 ? "" : "s")
			<< "." << CLR_RESET << std::endl;
	}

	mesh = Mesh(Vector3(), material, BVHNode(triangles));
}
