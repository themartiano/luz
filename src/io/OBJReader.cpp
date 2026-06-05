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
#include <sstream>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cmath>

static std::size_t	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset, Vector3 rotationDegrees, Vector3 scale, std::shared_ptr<Material> material);

namespace
{
	const double	DEGENERATE_TRIANGLE_EPSILON_SQUARED = 1e-24;
	const double	DEGREES_TO_RADIANS = 3.14159265358979323846 / 180.0;

	struct ObjFaceVertex
	{
		int	vertexIndex = -1;
		int	normalIndex = -1;
	};

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

	Vector3	transformNormal(Vector3 normal, Vector3 rotationDegrees, Vector3 scale)
	{
		normal.setX(scale.getX() == 0.0 ? normal.getX() : normal.getX() / scale.getX());
		normal.setY(scale.getY() == 0.0 ? normal.getY() : normal.getY() / scale.getY());
		normal.setZ(scale.getZ() == 0.0 ? normal.getZ() : normal.getZ() / scale.getZ());
		normal = rotateVertex(normal, rotationDegrees);
		if (Utilities::vectorLengthSquared(normal) <= DEGENERATE_TRIANGLE_EPSILON_SQUARED)
		{
			return (Vector3());
		}
		return (Utilities::normalize(normal));
	}

	Vector3	parseVectorValues(const std::string& value, const std::string& line)
	{
		double x;
		double y;
		double z;
		std::istringstream lineStream(value);

		if (!(lineStream >> x >> y >> z))
		{
			throw std::runtime_error("Invalid OBJ vector line: " + line);
		}
		return (Vector3(x, y, z));
	}

	int	resolveObjIndex(int index, std::size_t count, const std::string& token)
	{
		int resolvedIndex;

		if (index > 0)
		{
			resolvedIndex = index - 1;
		}
		else if (index < 0)
		{
			resolvedIndex = static_cast<int>(count) + index;
		}
		else
		{
			throw std::runtime_error("OBJ indices are 1-based: " + token);
		}
		if (resolvedIndex < 0 || static_cast<std::size_t>(resolvedIndex) >= count)
		{
			throw std::runtime_error("OBJ index out of range: " + token);
		}
		return (resolvedIndex);
	}

	ObjFaceVertex	parseFaceVertex(const std::string& token, std::size_t vertexCount, std::size_t normalCount)
	{
		ObjFaceVertex faceVertex;
		const std::size_t firstSlash = token.find('/');
		const std::size_t secondSlash = firstSlash == std::string::npos ? std::string::npos : token.find('/', firstSlash + 1);
		const std::string vertexPart = token.substr(0, firstSlash);

		if (vertexPart.empty())
		{
			throw std::runtime_error("OBJ face vertex is missing a position index: " + token);
		}
		faceVertex.vertexIndex = resolveObjIndex(std::stoi(vertexPart), vertexCount, token);
		if (secondSlash != std::string::npos && secondSlash + 1 < token.length())
		{
			faceVertex.normalIndex = resolveObjIndex(std::stoi(token.substr(secondSlash + 1)), normalCount, token);
		}

		return (faceVertex);
	}

	std::shared_ptr<Triangle>	buildTriangle(
		const std::vector<Vector3>& vertices,
		const std::vector<Vector3>& normals,
		const ObjFaceVertex& faceVertex0,
		const ObjFaceVertex& faceVertex1,
		const ObjFaceVertex& faceVertex2,
		Vector3 positionOffset,
		Vector3 rotationDegrees,
		Vector3 scale,
		std::shared_ptr<Material> material
	)
	{
		const Vector3 vertex0 = transformVertex(vertices.at(faceVertex0.vertexIndex), positionOffset, rotationDegrees, scale);
		const Vector3 vertex1 = transformVertex(vertices.at(faceVertex1.vertexIndex), positionOffset, rotationDegrees, scale);
		const Vector3 vertex2 = transformVertex(vertices.at(faceVertex2.vertexIndex), positionOffset, rotationDegrees, scale);

		if (faceVertex0.normalIndex >= 0 && faceVertex1.normalIndex >= 0 && faceVertex2.normalIndex >= 0)
		{
			return (std::make_shared<Triangle>(
				vertex0,
				vertex1,
				vertex2,
				transformNormal(normals.at(faceVertex0.normalIndex), rotationDegrees, scale),
				transformNormal(normals.at(faceVertex1.normalIndex), rotationDegrees, scale),
				transformNormal(normals.at(faceVertex2.normalIndex), rotationDegrees, scale),
				material
			));
		}

		return (std::make_shared<Triangle>(vertex0, vertex1, vertex2, material));
	}

	void	printMeshLoadProgress(const ObjLoadProgress& progress)
	{
		const std::size_t percent = progress.total == 0 ? 100 : (progress.loaded * 100) / progress.total;

		std::cout
			<< "\r" << CLR_CYAN << "Loading meshes: " << CLR_WHITE
			<< "[ " << percent << "% ]" << CLR_RESET << std::flush;
	}

	void	beginMeshLoadProgress(ObjLoadProgress& progress)
	{
		std::lock_guard<std::mutex> lock(progress.mutex);

		if (!progress.started)
		{
			progress.started = true;
			progress.startTime = std::chrono::steady_clock::now();
			printMeshLoadProgress(progress);
		}
	}

	void	finishMeshLoadProgress(ObjLoadProgress& progress, std::size_t skippedDegenerateTriangles)
	{
		std::lock_guard<std::mutex> lock(progress.mutex);

		if (progress.loaded < progress.total)
		{
			progress.loaded++;
		}
		progress.skippedDegenerateTriangles += skippedDegenerateTriangles;
		printMeshLoadProgress(progress);
		if (progress.loaded >= progress.total)
		{
			const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - progress.startTime;

			std::cout
				<< CLR_GREEN_BRIGHT << "\nMesh loading done! "
				<< CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << elapsed.count() << "s";
			if (progress.skippedDegenerateTriangles > 0)
			{
				std::cout
					<< CLR_BLUE_BRIGHT << ", skipped " << CLR_WHITE << progress.skippedDegenerateTriangles
					<< CLR_BLUE_BRIGHT << " degenerate triangle"
					<< (progress.skippedDegenerateTriangles == 1 ? "" : "s");
			}
			std::cout << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
		}
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
	return (readObj(fileName, positionOffset, rotationDegrees, scale, material, ObjReadOptions()));
}

Mesh	readObj(
	std::string fileName,
	Vector3 positionOffset,
	Vector3 rotationDegrees,
	Vector3 scale,
	std::shared_ptr<Material> material,
	const ObjReadOptions& options
)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		throw std::runtime_error("OBJ file could not be opened: " + fileName);
	}

	Clock	clock;
	Mesh	mesh;
	const bool useProgress = options.progress != nullptr && options.progress->total > 0;

	if (useProgress)
	{
		beginMeshLoadProgress(*options.progress);
	}
	else if (!options.quiet)
	{
		std::cout << CLR_YELLOW << "Parsing " << CLR_BLUE << fileName << CLR_YELLOW << "..." << CLR_RESET << std::endl;
	}

	clock.start();
	const std::size_t skippedDegenerateTriangles = parseObjFile(mesh, stream, positionOffset, rotationDegrees, scale, material);

	if (useProgress)
	{
		finishMeshLoadProgress(*options.progress, skippedDegenerateTriangles);
	}
	else if (!options.quiet)
	{
		if (skippedDegenerateTriangles > 0)
		{
			std::cout
				<< CLR_YELLOW << "Skipped " << CLR_WHITE << skippedDegenerateTriangles
				<< CLR_YELLOW << " degenerate OBJ triangle"
				<< (skippedDegenerateTriangles == 1 ? "" : "s")
				<< "." << CLR_RESET << std::endl;
		}
		std::cout << CLR_BLUE << fileName << CLR_GREEN_BRIGHT << " parsing done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << clock.elapsedS() << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
	}

	return (mesh);
}

static std::size_t	parseObjFile(Mesh& mesh, std::ifstream& stream, Vector3 positionOffset, Vector3 rotationDegrees, Vector3 scale, std::shared_ptr<Material> material)
{
	std::string line;
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
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
			vertices.push_back(parseVectorValues(line.substr(2), line));
		}
		else if (line.rfind("vn ", 0) == 0)
		{
			normals.push_back(parseVectorValues(line.substr(3), line));
		}
		else if (line.rfind("f ", 0) == 0)
		{
			std::istringstream lineStream(line.substr(2));
			std::vector<ObjFaceVertex> faceVertices;
			std::string token;

			while (lineStream >> token)
			{
				faceVertices.push_back(parseFaceVertex(token, vertices.size(), normals.size()));
			}
			if (faceVertices.size() < 3)
			{
				throw std::runtime_error("OBJ face has fewer than three vertices: " + line);
			}

			for (std::size_t index = 1; index + 1 < faceVertices.size(); index++)
			{
				const std::shared_ptr<Triangle> triangle = buildTriangle(
					vertices,
					normals,
					faceVertices.at(0),
					faceVertices.at(index),
					faceVertices.at(index + 1),
					positionOffset,
					rotationDegrees,
					scale,
					material
				);
				if (triangle->area() <= DEGENERATE_TRIANGLE_EPSILON_SQUARED)
				{
					skippedDegenerateTriangles++;
					continue;
				}
				triangles.push_back(triangle);
			}
		}
	} while (!stream.eof());

	mesh = Mesh(Vector3(), material, triangles);

	return (skippedDegenerateTriangles);
}
