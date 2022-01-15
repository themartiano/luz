#include "SceneFile/SceneFile.hpp"
#include "Utilities.hpp"
#include <fstream>

// Parses the [scene] section of a Scene file
void	SceneFile::internal::_readSceneSection(Scene& scene, std::ifstream& stream)
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
		Utilities::toLower(line);

		if (line.rfind("camera=", 0) != std::string::npos)
		{
			double pX, pY, pZ, dX, dY, dZ, aperture, focusDistance;
			int fov;

			if (sscanf(line.c_str(), "camera=(%lf,%lf,%lf),(%lf,%lf,%lf),%d,%lf,%lf\n", &pX, &pY, &pZ, &dX, &dY, &dZ, &fov, &aperture, &focusDistance) != EOF)
			{
				Camera camera;

				camera.setPosition(Vector3(pX, pY, pZ));
				camera.setDirection(Vector3(dX, dY, dZ));
				camera.setFOV(fov);
				camera.setAperture(aperture);
				camera.setFocusDistance(focusDistance);

				scene.addCamera(camera);
			}
		}
		else if (line.rfind("objects{", 0) != std::string::npos)
		{
			internal::_readObjectsSubSection(scene, stream);
		}
	} while (!stream.eof());
}
