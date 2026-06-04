#include "SceneFileInternal.hpp"
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
		std::string lowerLine = line;
		Utilities::toLower(lowerLine);

		if (lowerLine.rfind("resolution=", 0) != std::string::npos)
		{
			std::size_t x, y;

			if (sscanf(lowerLine.c_str(), "resolution=%zu,%zu", &x, &y) == 2)
			{
				scene.getImage()->setWidth(x);
				scene.getImage()->setHeight(y);
                scene.getImage()->initialize();
			}
		}
		else if (lowerLine.rfind("samples=", 0) != std::string::npos)
		{
			int samples;

			if (sscanf(lowerLine.c_str(), "samples=%d", &samples) == 1)
			{
				scene.setSampleCount(samples);
			}
		}
		else if (lowerLine.rfind("maxlightbounces=", 0) != std::string::npos)
		{
			int maxLightBounces;

			if (sscanf(lowerLine.c_str(), "maxlightbounces=%d", &maxLightBounces) == 1)
			{
				scene.setMaxLightBounces(maxLightBounces);
			}
		}
		else if (lowerLine.rfind("gamma=", 0) != std::string::npos)
		{
			int gammaCorrected;

			if (sscanf(lowerLine.c_str(), "gamma=%d", &gammaCorrected) == 1)
			{
				scene.setGammaCorrected(gammaCorrected);
			}
		}
		else if (lowerLine.rfind("bloom=", 0) != std::string::npos)
		{
			int useBloom;

			if (sscanf(lowerLine.c_str(), "bloom=%d", &useBloom) == 1)
			{
				scene.setBloom(useBloom);
			}
		}
		else if (lowerLine.rfind("outputfilename=", 0) != std::string::npos)
		{
			std::string strOutputFileName = line.substr(std::string("outputfilename=").size());
			if (!strOutputFileName.empty())
			{
				if (!Utilities::stringEndsWith(strOutputFileName, ".bmp"))
				{
					strOutputFileName += ".bmp";
				}

				scene.setDefaultRenderOutputFileName(strOutputFileName);
			}

		}
		else if (lowerLine.rfind("sky=", 0) != std::string::npos)
		{
			char sky[256];

			if (sscanf(lowerLine.c_str(), "sky=%255s", sky) == 1)
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
		else if (scene.getRenderSky() == SKY_ATMOSPHERE && lowerLine.rfind("atmosphere=", 0) != std::string::npos)
		{
			double sunAngle, earthRadius, atmosphereRadius, hR, hM, starsBrightness;
			int samples, lightSamples;

			if (sscanf(lowerLine.c_str(), "atmosphere=%lf,%lf,%lf,%lf,%lf,%d,%d,%lf", &sunAngle, &earthRadius, &atmosphereRadius, &hR, &hM, &samples, &lightSamples, &starsBrightness) == 8)
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
		else if (lowerLine.rfind("distanceblueness=", 0) != std::string::npos)
		{
			int distanceBlueness;

			if (sscanf(lowerLine.c_str(), "distanceblueness=%d", &distanceBlueness) == 1)
			{
				scene.setDistanceBlueness(distanceBlueness);
			}
		}
	} while (!stream.eof());
}
