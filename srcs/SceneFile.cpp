#include "SceneFile.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include "Vector3.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Cube.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Rectangle.hpp"
#include "Forms/Triangle.hpp"
#include "OBJReader.hpp"
#include "SkyTypes.hpp"
#include <fstream>
#include <memory>
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

		if (line.length() <= 0 || line.at(0) == '#')
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
        if (line.at(0) == '#')
        {
            continue;
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
        else if (line.rfind("outputfilename=", 0) != std::string::npos)
        {
            char* outputFileName = nullptr;

            if (sscanf(line.c_str(), "outputfilename=%ms\n", &outputFileName) != EOF)
            {
                std::string strOutputFileName(outputFileName);

                if (!Utilities::stringEndsWith(strOutputFileName, ".bmp"))
                {
                    strOutputFileName += ".bmp";
                }

                scene.setOutputFileName(strOutputFileName);
            }

            delete outputFileName;
        }
        else if (line.rfind("sky=", 0) != std::string::npos)
        {
            char* sky = nullptr;

            if (sscanf(line.c_str(), "sky=%ms\n", &sky) != EOF)
            {
                std::string skyStr = sky;
                Utilities::toLower(skyStr);

                if (skyStr == "atmosphere")
                {
                    scene.setRenderSky(SKY_ATMOSPHERE);
                }
                else if (skyStr == "linear")
                {
                    scene.setRenderSky(SKY_LINEAR);
                }
                else
                {
                    scene.setRenderSky(SKY_NONE);
                }
            }

            delete sky;
        }
        else if (scene.getRenderSky() == SKY_ATMOSPHERE && line.rfind("atmosphere=", 0) != std::string::npos)
        {
            double sunAngle, earthRadius, atmosphereRadius, hR, hM, starsBrightness;
            int samples, lightSamples;

            if (sscanf(line.c_str(), "atmosphere=%lf,%lf,%lf,%lf,%lf,%d,%d,%lf\n", &sunAngle, &earthRadius, &atmosphereRadius, &hR, &hM, &samples, &lightSamples, &starsBrightness) != EOF)
            {
                Atmosphere atmosphere;

                atmosphere.setSunAngle(sunAngle);
                atmosphere.setEarthRadius(earthRadius);
                atmosphere.setAtmosphereRadius(atmosphereRadius);
                atmosphere.setHR(hR);
                atmosphere.setHM(hM);
                atmosphere.setSamples(samples);
                atmosphere.setLightSamples(lightSamples);
                atmosphere.setStarsBrightness(starsBrightness);

                scene.setAtmosphere(atmosphere);
            }
        }
        else if (line.rfind("distanceblueness=", 0) != std::string::npos)
        {
            int distanceBlueness;

            if (sscanf(line.c_str(), "distanceblueness=%d\n", &distanceBlueness) != EOF)
            {
                scene.setDistanceBlueness(distanceBlueness);
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

		if (line.length() <= 0 || line.at(0) == '#')
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
            double mR, mG, mB, mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mLightIntensity;
            int mIsDieletric, mIsEmissive;

            if (sscanf(line.c_str(), "sphere=(%lf,%lf,%lf),%lf,(material=(%lf,%lf,%lf),%lf,%lf,%lf,%lf,%d,%d,%lf)\n", &pX, &pY, &pZ, &radius, &mR, &mG, &mB, &mOpacity, &mMetallic, &mAlbedo, &mReflectionFuzziness, &mIsDieletric, &mIsEmissive, &mLightIntensity) != EOF)
            {
                Sphere sphere;

                sphere.setPosition(Vector3(pX, pY, pZ));
                sphere.setRadius(radius);
                sphere.setMaterial(Material(Color(mR, mG, mB), mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mIsDieletric, mIsEmissive, mLightIntensity));

                scene.addHittable(std::make_shared<Sphere>(sphere));
            }
        }
        else if (line.rfind("cube=", 0) != std::string::npos)
        {
            double pX, pY, pZ, oX, oY, oZ, width, height, depth;
            double mR, mG, mB, mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mLightIntensity;
            int mIsDieletric, mIsEmissive;

            if (sscanf(line.c_str(), "cube=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,%lf,(material=(%lf,%lf,%lf),%lf,%lf,%lf,%lf,%d,%d,%lf)\n", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height, &depth, &mR, &mG, &mB, &mOpacity, &mMetallic, &mAlbedo, &mReflectionFuzziness, &mIsDieletric, &mIsEmissive, &mLightIntensity) != EOF)
            {
                Cube cube;

                cube.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
                cube.setWidth(width);
                cube.setHeight(height);
                cube.setDepth(depth);
                cube.setMaterial(Material(Color(mR, mG, mB), mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mIsDieletric, mIsEmissive, mLightIntensity));

                scene.addHittable(std::make_shared<Cube>(cube));
            }
        }
        else if (line.rfind("plane=", 0) != std::string::npos)
        {
            double y, oX, oY, oZ;
            double mR, mG, mB, mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mLightIntensity;
            int mIsDieletric, mIsEmissive;

            if (sscanf(line.c_str(), "plane=%lf,(%lf,%lf,%lf),(material=(%lf,%lf,%lf),%lf,%lf,%lf,%lf,%d,%d,%lf)\n", &y, &oX, &oY, &oZ, &mR, &mG, &mB, &mOpacity, &mMetallic, &mAlbedo, &mReflectionFuzziness, &mIsDieletric, &mIsEmissive, &mLightIntensity) != EOF)
            {
                Plane plane;

                plane.setY(y);
                plane.setOrientation(Vector3(oX, oY, oZ));
                plane.setMaterial(Material(Color(mR, mG, mB), mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mIsDieletric, mIsEmissive, mLightIntensity));

                scene.addHittable(std::make_shared<Plane>(plane));
            }
        }
        else if (line.rfind("rectangle=", 0) != std::string::npos)
        {
            double pX, pY, pZ, oX, oY, oZ, width, height;
            double mR, mG, mB, mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mLightIntensity;
            int mIsDieletric, mIsEmissive;

            if (sscanf(line.c_str(), "rectangle=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,(material=(%lf,%lf,%lf),%lf,%lf,%lf,%lf,%d,%d,%lf)\n", &pX, &pY, &pZ, &oX, &oY, &oZ, &width, &height, &mR, &mG, &mB, &mOpacity, &mMetallic, &mAlbedo, &mReflectionFuzziness, &mIsDieletric, &mIsEmissive, &mLightIntensity) != EOF)
            {
                Rectangle rectangle;

                rectangle.setTransform(Transform(Vector3(pX, pY, pZ), Vector3(oX, oY, oZ), Vector3(1.0, 1.0, 1.0)));
                rectangle.setWidth(width);
                rectangle.setHeight(height);
                rectangle.setMaterial(Material(Color(mR, mG, mB), mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mIsDieletric, mIsEmissive, mLightIntensity));

                scene.addHittable(std::make_shared<Rectangle>(rectangle));
            }
        }
        else if (line.rfind("triangle=", 0) != std::string::npos)
        {
            double v0X, v0Y, v0Z, v1X, v1Y, v1Z, v2X, v2Y, v2Z;
            double mR, mG, mB, mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mLightIntensity;
            int mIsDieletric, mIsEmissive;

            if (sscanf(line.c_str(), "triangle=(%lf,%lf,%lf),(%lf,%lf,%lf),(%lf,%lf,%lf),(material=(%lf,%lf,%lf),%lf,%lf,%lf,%lf,%d,%d,%lf)\n", &v0X, &v0Y, &v0Z, &v1X, &v1Y, &v1Z, &v2X, &v2Y, &v2Z, &mR, &mG, &mB, &mOpacity, &mMetallic, &mAlbedo, &mReflectionFuzziness, &mIsDieletric, &mIsEmissive, &mLightIntensity) != EOF)
            {
                Triangle triangle;

                triangle.setVertex0(Vector3(v0X, v0Y, v0Z));
                triangle.setVertex1(Vector3(v1X, v1Y, v1Z));
                triangle.setVertex2(Vector3(v2X, v2Y, v2Z));
                triangle.setMaterial(Material(Color(mR, mG, mB), mOpacity, mMetallic, mAlbedo, mReflectionFuzziness, mIsDieletric, mIsEmissive, mLightIntensity));

                scene.addHittable(std::make_shared<Triangle>(triangle));
            }
        }
        else if (line.rfind("obj=", 0) != std::string::npos)
        {
            char* objFileName = nullptr;

            if (sscanf(line.c_str(), "obj=%ms\n", &objFileName) != EOF)
            {
                readObj(scene, objFileName);
            }

            delete objFileName;
        }
	} while (!stream.eof());
}
