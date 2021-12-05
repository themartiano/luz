#include "OBJReader.hpp"
#include "Hittable.hpp"
#include "BVHNode.hpp"
#include "Forms/Triangle.hpp"
#include "Vector3.hpp"
#include "Utilities.hpp"
#include "ANSIColors.hpp"
#include <memory>
#include <vector>
#include <fstream>

// Search and read / parse the obj file named 'fileName' (current directory)
void    readObj(Scene& scene, std::string fileName)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		std::cerr << CLR_RED << "The specified file could not be opened." << CLR_RESET << std::endl;
		exit(1);
	}

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

			triangles.push_back(std::make_shared<Triangle>( // Maybe try ordering the vertices from smallest => medium => biggest value or something?
				vertices[positions[0] - 1],
				vertices[positions[1] - 1],
				vertices[positions[2] - 1],
				Material(Color(randomDouble(), randomDouble(), randomDouble()), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
			));
		}
	} while (!stream.eof());

	for (std::size_t i = 0; i < triangles.size(); i++)
	{
		scene.addHittable(triangles[i]);
	}
	// scene.addHittable(std::make_shared<BVHNode>(triangles));
}
