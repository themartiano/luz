#include "SceneFile.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include "Vector3.hpp"
#include <fstream>
#include <stdio.h>

static void	readSettingsSection(Scene& scene, std::ifstream& stream);
static void	readSceneSection(Scene& scene, std::ifstream& stream);
static void	readObjectsSubSection(Scene& scene, std::ifstream& stream);

// Searches and reads / parses the Scene file named 'fileName' into 'Scene' (searches in the current directory)
void   readSceneFile(Scene& scene, std::string fileName)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		std::cerr << CLR_RED << "The specified file could not be opened." << CLR_RESET << std::endl;
		exit(1);
	}

	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			continue;
		}
        Utilities::toLower(line);

        if (line.rfind("[settings]", 0) != std::string::npos)
        {
            readSettingsSection(scene, stream);
        }
        else if (line.rfind("[scene]", 0) != std::string::npos)
        {
            readSceneSection(scene, stream);
        }
        else if (line.rfind("[materials]", 0) != std::string::npos)
        {
            //readMaterialsSection(scene, stream);
        }
	} while (!stream.eof());
}

// Parses the [settings] section of a Scene file
static void	readSettingsSection(Scene& scene, std::ifstream& stream)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			break;
		}
        Utilities::toLower(line);

        if (line.rfind("resolution=", 0) != std::string::npos)
        {
            double x, y;

            if (sscanf(line.c_str(), "resolution=%lf,%lf\n", &x, &y) != EOF)
            {
                scene.setXResolution(x);
                scene.setYResolution(y);
            }
        }
        else if (line.rfind("samples=", 0) != std::string::npos)
        {
            int samples;

            if (sscanf(line.c_str(), "samples=%d\n", &samples) != EOF)
            {
                scene.setSampleCount(samples);
            }
        }
        else if (line.rfind("maxlightbounces=", 0) != std::string::npos)
        {
            int maxLightBounces;

            if (sscanf(line.c_str(), "maxlightbounces=%d\n", &maxLightBounces) != EOF)
            {
                scene.setMaxLightBounces(maxLightBounces);
            }
        }
        else if (line.rfind("gamma=", 0) != std::string::npos)
        {
            int gammaCorrected;

            if (sscanf(line.c_str(), "gamma=%d\n", &gammaCorrected) != EOF)
            {
                scene.setGammaCorrected(gammaCorrected);
            }
        }
	} while (!stream.eof());
}

// Parses the [scene] section of a Scene file
static void	readSceneSection(Scene& scene, std::ifstream& stream)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			break;
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
            readObjectsSubSection(scene, stream);
        }
	} while (!stream.eof());
}

// Parses the 'objects' sub-section of a Scene file
static void	readObjectsSubSection(Scene& scene, std::ifstream& stream)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			continue;
		}
        if (line == "}")
		{
			break;
		}
        Utilities::toLower(line);

        if (line.rfind("sphere=", 0) != std::string::npos)
        {
            double pX, pY, pZ, radius;
            std::string materialName;

            if (sscanf(line.c_str(), "sphere=(%lf,%lf,%lf),%s,%lf\n", &pX, &pY, &pZ, &materialName.c_str(), &radius) != EOF)
            {
                Sphere sphere;

                sphere.

                scene.addHittable(sphere);
            }
        }
	} while (!stream.eof());
}
