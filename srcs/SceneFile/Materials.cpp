#include "SceneFile/SceneFile.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Emissive.hpp"
#include "Utilities.hpp"
#include <fstream>

// Parses a Material from a Scene file
std::shared_ptr<Material>	SceneFile::internal::_readMaterialSubSection(std::ifstream& stream)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0 || line.at(0) == '#')
		{
			continue;
		}
		if (line == "]")
		{
			break;
		}
		Utilities::toLower(line);

		if (line.rfind("lambertian=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(line.c_str(), "lambertian=(%lf,%lf,%lf)\n", &r, &g, &b) != EOF)
			{
				return (std::make_shared<Lambertian>(Color(r, g, b)));
			}
		}
		else if (line.rfind("metal=", 0) != std::string::npos)
		{
			double r, g, b, reflectionFuzziness;

			if (sscanf(line.c_str(), "metal=(%lf,%lf,%lf),%lf\n", &r, &g, &b, &reflectionFuzziness) != EOF)
			{
				return (std::make_shared<Metal>(Color(r, g, b), reflectionFuzziness));
			}
		}
		else if (line.rfind("dielectric=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(line.c_str(), "dielectric=(%lf,%lf,%lf)\n", &r, &g, &b) != EOF)
			{
				return (std::make_shared<Dielectric>(Color(r, g, b)));
			}
		}
		else if (line.rfind("emissive=", 0) != std::string::npos)
		{
			double r, g, b, lightIntensity;

			if (sscanf(line.c_str(), "emissive=(%lf,%lf,%lf),%lf\n", &r, &g, &b, &lightIntensity) != EOF)
			{
				return (std::make_shared<Emissive>(Color(r, g, b), lightIntensity));
			}
		}
	} while (!stream.eof());

	return (std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6)));
}
