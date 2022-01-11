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

static void	parseObjFile(Scene& scene, std::ifstream& stream, bool& done, std::size_t& currentLine);

// Search and read / parse the obj file named 'fileName' (current directory)
void	readObj(Scene& scene, std::string fileName)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		std::cerr << CLR_RED << "The specified file could not be opened." << CLR_RESET << std::endl;
		exit(1);
	}

	Clock		clock;
	std::size_t	totalLines = std::count(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(), '\n');
	std::size_t	currentLine = 0;
	bool		done = false;

	parseObjFile(scene, stream, done, currentLine);
}

static void	parseObjFile(Scene& scene, std::ifstream& stream, bool& done, std::size_t& currentLine)
{
	std::string line;
	std::vector<Vector3> vertices;
	std::vector<std::shared_ptr<Hittable>> triangles;

	do
	{
		getline(stream, line);
		currentLine++;

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
				vertices[positions[0] - 1],
				vertices[positions[1] - 1],
				vertices[positions[2] - 1],
				//Material(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble()), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
				Material(Color(0.3, 0.3, 0.3), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
			));
		}
	} while (!stream.eof());

	scene.addHittable(std::make_shared<BVHNode>(triangles));
}
