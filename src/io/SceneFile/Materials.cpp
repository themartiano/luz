#include "SceneFileInternal.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Emissive.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <stdexcept>

// Parses a Material from a Scene file
std::shared_ptr<Material>	SceneFile::internal::_readMaterialSubSection(std::ifstream& stream)
{
	std::string line;
	std::shared_ptr<Material> material;

	do
	{
		getline(stream, line);

		if (line.length() <= 0 || line.at(0) == '#')
		{
			continue;
		}
		if (line == "]")
		{
			if (!material)
			{
				throw std::runtime_error("Material block ended before a material was defined.");
			}
			return (material);
		}
		std::string lowerLine = line;
		Utilities::toLower(lowerLine);

		if (lowerLine.rfind("lambertian=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(lowerLine.c_str(), "lambertian=(%lf,%lf,%lf)", &r, &g, &b) != 3)
			{
				throw std::runtime_error("Invalid lambertian material: " + line);
			}
			if (material)
			{
				throw std::runtime_error("Material block defines more than one material.");
			}
			material = std::make_shared<Lambertian>(Color(r, g, b));
		}
		else if (lowerLine.rfind("metal=", 0) != std::string::npos)
		{
			double r, g, b, reflectionFuzziness;

			if (sscanf(lowerLine.c_str(), "metal=(%lf,%lf,%lf),%lf", &r, &g, &b, &reflectionFuzziness) != 4)
			{
				throw std::runtime_error("Invalid metal material: " + line);
			}
			if (material)
			{
				throw std::runtime_error("Material block defines more than one material.");
			}
			material = std::make_shared<Metal>(Color(r, g, b), reflectionFuzziness);
		}
		else if (lowerLine.rfind("dielectric=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(lowerLine.c_str(), "dielectric=(%lf,%lf,%lf)", &r, &g, &b) != 3)
			{
				throw std::runtime_error("Invalid dielectric material: " + line);
			}
			if (material)
			{
				throw std::runtime_error("Material block defines more than one material.");
			}
			material = std::make_shared<Dielectric>(Color(r, g, b));
		}
		else if (lowerLine.rfind("emissive=", 0) != std::string::npos)
		{
			double r, g, b, lightIntensity;

			if (sscanf(lowerLine.c_str(), "emissive=(%lf,%lf,%lf),%lf", &r, &g, &b, &lightIntensity) != 4)
			{
				throw std::runtime_error("Invalid emissive material: " + line);
			}
			if (material)
			{
				throw std::runtime_error("Material block defines more than one material.");
			}
			material = std::make_shared<Emissive>(Color(r, g, b), lightIntensity);
		}
		else
		{
			throw std::runtime_error("Unknown material line: " + line);
		}
	} while (!stream.eof());

	throw std::runtime_error("Material block is missing a closing ].");
}
