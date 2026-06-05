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

static void	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset, std::shared_ptr<Material> material);

namespace
{
	const double	DEGENERATE_TRIANGLE_EPSILON_SQUARED = 1e-24;
}

// Calls the actual 'readObj' function with a zeroed offset position
Mesh	readObj(std::string fileName)
{
	return (readObj(fileName, Vector3(0.0, 0.0, 0.0), std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))));
}

// Search and read / parse the obj file named 'fileName' (current directory)
Mesh	readObj(std::string fileName, Vector3 positionOffset, std::shared_ptr<Material> material)
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
	parseObjFile(mesh, stream, positionOffset, material);

	std::cout << CLR_BLUE << fileName << CLR_GREEN_BRIGHT << " parsing done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << clock.elapsedS() << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;

	return (mesh);
}

static void	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset, std::shared_ptr<Material> material)
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

			Vector3 vertex0 = vertices[positions[0] - 1] + positionOffset;
			Vector3 vertex1 = vertices[positions[1] - 1] + positionOffset;
			Vector3 vertex2 = vertices[positions[2] - 1] + positionOffset;
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
