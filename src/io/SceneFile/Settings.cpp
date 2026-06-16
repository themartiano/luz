#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	void	requireBinarySetting(int value, const std::string& name)
	{
		if (value != 0 && value != 1)
		{
			throw std::runtime_error(name + " must be 0 or 1.");
		}
	}

	std::string	settingValue(const std::string& line, const std::string& message)
	{
		const std::size_t separator = line.find('=');

		if (separator == std::string::npos || separator == line.length() - 1)
		{
			throw std::runtime_error(message);
		}
		return (SceneFile::internal::_trim(line.substr(separator + 1)));
	}

	double	parseFiniteDouble(const std::string& value, const std::string& label)
	{
		std::size_t parsed = 0;
		const std::string trimmed = SceneFile::internal::_trim(value);
		const double result = std::stod(trimmed, &parsed);

		if (parsed != trimmed.length() || !std::isfinite(result))
		{
			throw std::runtime_error(label + " must be finite.");
		}
		return (result);
	}

	std::vector<std::string>	splitCommaList(const std::string& value)
	{
		std::vector<std::string> parts;
		std::size_t start = 0;

		while (start <= value.length())
		{
			const std::size_t comma = value.find(',', start);
			const std::size_t end = comma == std::string::npos ? value.length() : comma;

			parts.push_back(SceneFile::internal::_trim(value.substr(start, end - start)));
			if (comma == std::string::npos)
			{
				break;
			}
			start = comma + 1;
		}
		return (parts);
	}
}

// Parses the [settings] section of a Scene file
void	SceneFile::internal::_readSettingsSection(Scene& scene, std::ifstream& stream, SceneFileContext& context)
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
		else if (
			lowerLine.rfind("adaptive=", 0) != std::string::npos
			|| lowerLine.rfind("adaptivesampling=", 0) != std::string::npos
			|| lowerLine.rfind("adaptive_sampling=", 0) != std::string::npos
		)
		{
			int adaptiveSampling;

			if (sscanf(lowerLine.c_str(), "%*[^=]=%d", &adaptiveSampling) != 1)
			{
				throw std::runtime_error("Invalid adaptive setting. Use adaptive=0 or adaptive=1.");
			}
			requireBinarySetting(adaptiveSampling, "adaptive");
			scene.setAdaptiveSampling(adaptiveSampling);
		}
		else if (
			lowerLine.rfind("adaptiveminsamples=", 0) != std::string::npos
			|| lowerLine.rfind("adaptive_min_samples=", 0) != std::string::npos
		)
		{
			int adaptiveMinSamples;

			if (sscanf(lowerLine.c_str(), "%*[^=]=%d", &adaptiveMinSamples) != 1)
			{
				throw std::runtime_error("Invalid adaptive minimum samples setting. Use adaptiveminsamples=N.");
			}
			scene.setAdaptiveMinSamples(adaptiveMinSamples);
		}
		else if (
			lowerLine.rfind("adaptivethreshold=", 0) != std::string::npos
			|| lowerLine.rfind("adaptive_threshold=", 0) != std::string::npos
		)
		{
			double adaptiveThreshold;

			if (sscanf(lowerLine.c_str(), "%*[^=]=%lf", &adaptiveThreshold) != 1)
			{
				throw std::runtime_error("Invalid adaptive threshold setting. Use adaptivethreshold=F.");
			}
			scene.setAdaptiveThreshold(adaptiveThreshold);
		}
		else if (
			lowerLine.rfind("adaptivecheckinterval=", 0) != std::string::npos
			|| lowerLine.rfind("adaptive_check_interval=", 0) != std::string::npos
		)
		{
			int adaptiveCheckInterval;

			if (sscanf(lowerLine.c_str(), "%*[^=]=%d", &adaptiveCheckInterval) != 1)
			{
				throw std::runtime_error("Invalid adaptive check interval setting. Use adaptivecheckinterval=N.");
			}
			scene.setAdaptiveCheckInterval(adaptiveCheckInterval);
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
		else if (
			lowerLine.rfind("tonemapping=", 0) != std::string::npos
			|| lowerLine.rfind("tone_mapping=", 0) != std::string::npos
		)
		{
			int toneMapped;

			if (sscanf(lowerLine.c_str(), "%*[^=]=%d", &toneMapped) != 1)
			{
				throw std::runtime_error("Invalid tonemapping setting. Use tonemapping=0 or tonemapping=1.");
			}
			requireBinarySetting(toneMapped, "tonemapping");
			scene.setToneMapped(toneMapped);
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
		else if (lowerLine.rfind("exposure=", 0) != std::string::npos)
		{
			double exposure;

			if (sscanf(lowerLine.c_str(), "exposure=%lf", &exposure) != 1)
			{
				throw std::runtime_error("Invalid exposure setting. Use exposure=F.");
			}
			scene.setExposure(exposure);
		}
		else if (lowerLine.rfind("contrast=", 0) != std::string::npos)
		{
			double contrast;

			if (sscanf(lowerLine.c_str(), "contrast=%lf", &contrast) != 1)
			{
				throw std::runtime_error("Invalid contrast setting. Use contrast=F.");
			}
			scene.setContrast(contrast);
		}
		else if (lowerLine.rfind("denoise=", 0) != std::string::npos)
		{
			int useDenoise;

			if (sscanf(lowerLine.c_str(), "denoise=%d", &useDenoise) != 1)
			{
				throw std::runtime_error("Invalid denoise setting. Use denoise=0 or denoise=1.");
			}
			requireBinarySetting(useDenoise, "denoise");
			scene.setDenoise(useDenoise);
		}
		else if (
			lowerLine.rfind("denoiseoutputfilename=", 0) != std::string::npos
			|| lowerLine.rfind("denoiseoutput=", 0) != std::string::npos
			|| lowerLine.rfind("denoise_output=", 0) != std::string::npos
		)
		{
			const std::size_t separator = line.find('=');

			if (separator == std::string::npos || separator == line.length() - 1)
			{
				throw std::runtime_error("Invalid denoise output setting. Use denoiseoutputfilename=PATH.");
			}
			scene.setDenoiseOutputFileName(line.substr(separator + 1));
		}
		else if (lowerLine.rfind("outputfilename=", 0) != std::string::npos)
		{
			std::string strOutputFileName = line.substr(std::string("outputfilename=").size());
			if (strOutputFileName.empty())
			{
				throw std::runtime_error("Invalid outputfilename setting. Use outputfilename=PATH.");
			}
			std::filesystem::path outputPath(strOutputFileName);
			std::string lowerExtension = outputPath.extension().string();
			Utilities::toLower(lowerExtension);
			if (lowerExtension.empty())
			{
				strOutputFileName += ".bmp";
			}
			else if (
				lowerExtension != ".bmp"
				&& lowerExtension != ".png"
				&& lowerExtension != ".tiff"
			)
			{
				throw std::runtime_error("Invalid outputfilename setting. Use a .bmp, .png, or .tiff path.");
			}
			scene.setDefaultRenderOutputFileName(strOutputFileName);
		}
		else if (lowerLine.rfind("sky=", 0) != std::string::npos)
		{
			char sky[256];

			if (sscanf(lowerLine.c_str(), "sky=%255s", sky) != 1)
			{
				throw std::runtime_error("Invalid sky setting. Use sky=none, sky=linear, sky=atmosphere, or sky=environment.");
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
			else if (skyStr == "environment")
			{
				scene.setRenderSky(SKY_ENVIRONMENT);
			}
			else
			{
				throw std::runtime_error("Unknown sky setting: " + line);
			}
		}
		else if (
			lowerLine.rfind("environment=", 0) != std::string::npos
			|| lowerLine.rfind("environmentmap=", 0) != std::string::npos
			|| lowerLine.rfind("environment_map=", 0) != std::string::npos
			|| lowerLine.rfind("backgroundimage=", 0) != std::string::npos
			|| lowerLine.rfind("background_image=", 0) != std::string::npos
		)
		{
			const std::vector<std::string> parts = splitCommaList(settingValue(
				line,
				"Invalid environment setting. Use environment=PATH[,STRENGTH[,ROTATION_DEGREES]]."
			));

			if (parts.empty() || parts[0].empty() || parts.size() > 3)
			{
				throw std::runtime_error("Invalid environment setting. Use environment=PATH[,STRENGTH[,ROTATION_DEGREES]].");
			}
			scene.setEnvironmentMap(std::make_shared<EnvironmentMap>(
				EnvironmentMap::load(_resolveAssetPath(context.baseDirectory, parts[0]))
			));
			if (parts.size() >= 2 && !parts[1].empty())
			{
				scene.setEnvironmentStrength(parseFiniteDouble(parts[1], "Environment strength"));
			}
			if (parts.size() >= 3 && !parts[2].empty())
			{
				scene.setEnvironmentRotation(parseFiniteDouble(parts[2], "Environment rotation"));
			}
			scene.setRenderSky(SKY_ENVIRONMENT);
		}
		else if (
			lowerLine.rfind("environmentstrength=", 0) != std::string::npos
			|| lowerLine.rfind("environment_strength=", 0) != std::string::npos
			|| lowerLine.rfind("worldstrength=", 0) != std::string::npos
			|| lowerLine.rfind("world_strength=", 0) != std::string::npos
		)
		{
			scene.setEnvironmentStrength(parseFiniteDouble(
				settingValue(line, "Invalid environment strength setting. Use environmentstrength=F."),
				"Environment strength"
			));
		}
		else if (
			lowerLine.rfind("environmentrotation=", 0) != std::string::npos
			|| lowerLine.rfind("environment_rotation=", 0) != std::string::npos
			|| lowerLine.rfind("worldrotation=", 0) != std::string::npos
			|| lowerLine.rfind("world_rotation=", 0) != std::string::npos
		)
		{
			scene.setEnvironmentRotation(parseFiniteDouble(
				settingValue(line, "Invalid environment rotation setting. Use environmentrotation=DEGREES."),
				"Environment rotation"
			));
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
