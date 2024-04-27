#include "SceneFile/SceneFile.hpp"
#include "Utilities.hpp"
#include <fstream>

// Parses the [settings] section of a Scene file
void	SceneFile::internal::_readSettingsSection(Scene& scene, std::ifstream& stream)
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
				scene.getImage()->setWidth(x);
				scene.getImage()->setHeight(y);
                scene.getImage()->initialize();
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
			char outputFileName[256];

			if (sscanf(line.c_str(), "outputfilename=%s\n", outputFileName) != EOF)
			{
				std::string strOutputFileName(outputFileName);

				if (!Utilities::stringEndsWith(strOutputFileName, ".bmp"))
				{
					strOutputFileName += ".bmp";
				}

				scene.setDefaultRenderOutputFileName(strOutputFileName);
			}

		}
		else if (line.rfind("sky=", 0) != std::string::npos)
		{
			char sky[256];

			if (sscanf(line.c_str(), "sky=%s\n", sky) != EOF)
			{
				std::string skyStr(sky);
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
