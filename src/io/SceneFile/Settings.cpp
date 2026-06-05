#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
	void	requireBinarySetting(int value, const std::string& name)
	{
		if (value != 0 && value != 1)
		{
			throw std::runtime_error(name + " must be 0 or 1.");
		}
	}
}

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
			long long x, y;

			if (sscanf(lowerLine.c_str(), "resolution=%lld,%lld", &x, &y) != 2)
			{
				throw std::runtime_error("Invalid resolution setting. Use resolution=WIDTH,HEIGHT.");
			}
			if (x <= 0 || y <= 0)
			{
				throw std::runtime_error("Scene resolution dimensions must be positive.");
			}
			scene.getImage()->setWidth(static_cast<std::size_t>(x));
			scene.getImage()->setHeight(static_cast<std::size_t>(y));
			scene.getImage()->initialize();
		}
		else if (lowerLine.rfind("samples=", 0) != std::string::npos)
		{
			int samples;

			if (sscanf(lowerLine.c_str(), "samples=%d", &samples) != 1)
			{
				throw std::runtime_error("Invalid samples setting. Use samples=N.");
			}
			scene.setSampleCount(samples);
		}
		else if (lowerLine.rfind("maxlightbounces=", 0) != std::string::npos)
		{
			int maxLightBounces;

			if (sscanf(lowerLine.c_str(), "maxlightbounces=%d", &maxLightBounces) != 1)
			{
				throw std::runtime_error("Invalid maxlightbounces setting. Use maxlightbounces=N.");
			}
			scene.setMaxLightBounces(maxLightBounces);
		}
		else if (lowerLine.rfind("gamma=", 0) != std::string::npos)
		{
			int gammaCorrected;

			if (sscanf(lowerLine.c_str(), "gamma=%d", &gammaCorrected) != 1)
			{
				throw std::runtime_error("Invalid gamma setting. Use gamma=0 or gamma=1.");
			}
			requireBinarySetting(gammaCorrected, "gamma");
			scene.setGammaCorrected(gammaCorrected);
		}
		else if (lowerLine.rfind("bloom=", 0) != std::string::npos)
		{
			int useBloom;

			if (sscanf(lowerLine.c_str(), "bloom=%d", &useBloom) != 1)
			{
				throw std::runtime_error("Invalid bloom setting. Use bloom=0 or bloom=1.");
			}
			requireBinarySetting(useBloom, "bloom");
			scene.setBloom(useBloom);
		}
		else if (lowerLine.rfind("outputfilename=", 0) != std::string::npos)
		{
			std::string strOutputFileName = line.substr(std::string("outputfilename=").size());
			if (strOutputFileName.empty())
			{
				throw std::runtime_error("Invalid outputfilename setting. Use outputfilename=PATH.");
			}
			std::string lowerOutputFileName = strOutputFileName;
			Utilities::toLower(lowerOutputFileName);
			if (
				!Utilities::stringEndsWith(lowerOutputFileName, ".bmp")
				&& !Utilities::stringEndsWith(lowerOutputFileName, ".tiff")
				&& !Utilities::stringEndsWith(lowerOutputFileName, ".tif")
			)
			{
				strOutputFileName += ".bmp";
			}
			scene.setDefaultRenderOutputFileName(strOutputFileName);
		}
		else if (lowerLine.rfind("sky=", 0) != std::string::npos)
		{
			char sky[256];

			if (sscanf(lowerLine.c_str(), "sky=%255s", sky) != 1)
			{
				throw std::runtime_error("Invalid sky setting. Use sky=none, sky=linear, or sky=atmosphere.");
			}
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
			else if (skyStr == "none")
			{
				scene.setRenderSky(SKY_NONE);
			}
			else
			{
				throw std::runtime_error("Unknown sky setting: " + line);
			}
		}
		else if (
			lowerLine.rfind("background=", 0) != std::string::npos
			|| lowerLine.rfind("backgroundcolor=", 0) != std::string::npos
			|| lowerLine.rfind("background_color=", 0) != std::string::npos
		)
		{
			const std::size_t separator = line.find('=');

			if (separator == std::string::npos || separator == line.length() - 1)
			{
				throw std::runtime_error("Invalid background setting. Use background=(R,G,B).");
			}
			scene.setBackgroundColor(_parseColorValue(line.substr(separator + 1), "background"));
		}
		else if (lowerLine.rfind("atmosphere=", 0) != std::string::npos)
		{
			double sunAngle, earthRadius, atmosphereRadius, hR, hM, starsBrightness;
			int samples, lightSamples;

			if (scene.getRenderSky() != SKY_ATMOSPHERE)
			{
				throw std::runtime_error("Atmosphere settings require sky=atmosphere.");
			}
			if (sscanf(lowerLine.c_str(), "atmosphere=%lf,%lf,%lf,%lf,%lf,%d,%d,%lf", &sunAngle, &earthRadius, &atmosphereRadius, &hR, &hM, &samples, &lightSamples, &starsBrightness) != 8)
			{
				throw std::runtime_error("Invalid atmosphere setting. Use atmosphere=SUN,EARTH_RADIUS,ATMOSPHERE_RADIUS,HR,HM,SAMPLES,LIGHT_SAMPLES,STARS.");
			}
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
		else if (lowerLine.rfind("distanceblueness=", 0) != std::string::npos)
		{
			int distanceBlueness;

			if (sscanf(lowerLine.c_str(), "distanceblueness=%d", &distanceBlueness) != 1)
			{
				throw std::runtime_error("Invalid distanceblueness setting. Use distanceblueness=0 or distanceblueness=1.");
			}
			requireBinarySetting(distanceBlueness, "distanceblueness");
			scene.setDistanceBlueness(distanceBlueness);
		}
		else
		{
			throw std::runtime_error("Unknown scene setting: " + line);
		}
	} while (!stream.eof());
}
