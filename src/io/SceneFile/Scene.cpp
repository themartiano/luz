#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <stdexcept>

// Parses the [scene] section of a Scene file
void	SceneFile::internal::_readSceneSection(Scene& scene, std::ifstream& stream, const std::filesystem::path& baseDirectory)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			break;
		}
		if (line.at(0) == '#')
		{
			continue;
		}
		std::string lowerLine = line;
		Utilities::toLower(lowerLine);

		if (lowerLine.rfind("camera=", 0) != std::string::npos)
		{
			double pX, pY, pZ, dX, dY, dZ, aperture, focusDistance;
			int fov;

			if (sscanf(lowerLine.c_str(), "camera=(%lf,%lf,%lf),(%lf,%lf,%lf),%d,%lf,%lf", &pX, &pY, &pZ, &dX, &dY, &dZ, &fov, &aperture, &focusDistance) == 9)
			{
				Camera camera;

				camera.setPosition(Vector3(pX, pY, pZ));
				camera.setDirection(Vector3(dX, dY, dZ));
				camera.setFOV(fov);
				camera.setAperture(aperture);
				camera.setFocusDistance(focusDistance);

				scene.addCamera(camera);
			}
			else
			{
				throw std::runtime_error("Invalid camera line: " + line);
			}
		}
		else if (lowerLine.rfind("objects{", 0) != std::string::npos)
		{
			internal::_readObjectsSubSection(scene, stream, baseDirectory);
		}
		else
		{
			throw std::runtime_error("Unknown scene line: " + line);
		}
	} while (!stream.eof());
}
