#include "OBJReader.hpp"
#include "Hittable.hpp"
#include "BVHNode.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Mesh.hpp"
#include "Vector3.hpp"
#include "Utilities.hpp"
#include "ANSIColors.hpp"
#include "Clock.hpp"
#include "Materials/Lambertian.hpp"
#include <memory>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <unistd.h>

static void	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset);

// Calls the actual 'readObj' function with a zeroed offset position
Mesh	readObj(std::string fileName)
{
	return (readObj(fileName, Vector3(0.0, 0.0, 0.0)));
}

// Search and read / parse the obj file named 'fileName' (current directory)
Mesh	readObj(std::string fileName, Vector3 positionOffset)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		std::cerr << CLR_RED << "The specified file could not be opened." << CLR_RESET << std::endl;
		exit(1);
	}

	std::cout << CLR_YELLOW << "Parsing " << CLR_BLUE << fileName << CLR_YELLOW << "..." << CLR_RESET << std::endl;

	Clock	clock;

	Mesh	mesh;
	parseObjFile(mesh, stream, positionOffset);

	double elapsedS = clock.stop();
	std::cout << CLR_BLUE << fileName << CLR_GREEN_BRIGHT << " parsing done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << elapsedS << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;

	return (mesh);
}

static void	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset)
{
	std::string line;
	std::vector<Vector3> vertices;
	std::vector<std::shared_ptr<Hittable>> triangles;

	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			continue;
		}

		if (line.at(0) == 'v')
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
		else if (line.at(0) == 'f')
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

			triangles.push_back(std::make_shared<Triangle>(
				vertices[positions[0] - 1] + positionOffset,
				vertices[positions[1] - 1] + positionOffset,
				vertices[positions[2] - 1] + positionOffset,
				//Material(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble()), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
				//std::make_shared<Dielectric>(Color(0.8, 0.8, 0.8))
				std::make_shared<Lambertian>(Color(0.3, 0.3, 0.3))
			));
		}
	} while (!stream.eof());

	mesh = Mesh(Vector3(), std::make_shared<Lambertian>(Color(0.3, 0.3, 0.3)), BVHNode(triangles));
}
