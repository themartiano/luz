#include "ColorScience.hpp"
#include "ColorManagement.hpp"
#include "LightUnits.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
	double	gaussian(double value)
	{
		return (std::exp(-0.5 * value * value));
	}

	double	cieX(double wavelength)
	{
		const double t1 = (wavelength - 442.0) * (wavelength < 442.0 ? 0.0624 : 0.0374);
		const double t2 = (wavelength - 599.8) * (wavelength < 599.8 ? 0.0264 : 0.0323);
		const double t3 = (wavelength - 501.1) * (wavelength < 501.1 ? 0.0490 : 0.0382);

		return (0.362 * gaussian(t1) + 1.056 * gaussian(t2) - 0.065 * gaussian(t3));
	}

	double	cieY(double wavelength)
	{
		const double t1 = (wavelength - 568.8) * (wavelength < 568.8 ? 0.0213 : 0.0247);
		const double t2 = (wavelength - 530.9) * (wavelength < 530.9 ? 0.0613 : 0.0322);

		return (0.821 * gaussian(t1) + 0.286 * gaussian(t2));
	}

	double	cieZ(double wavelength)
	{
		const double t1 = (wavelength - 437.0) * (wavelength < 437.0 ? 0.0845 : 0.0278);
		const double t2 = (wavelength - 459.0) * (wavelength < 459.0 ? 0.0385 : 0.0725);

		return (1.217 * gaussian(t1) + 0.681 * gaussian(t2));
	}

	Color	xyzToACEScg(double x, double y, double z)
	{
		return (ColorManagement::acescgFromXYZ(Color(x, y, z)));
	}

	Color	normalizeVisibleColor(Color color)
	{
		const double red = std::isfinite(color.getRed()) ? std::max(0.0, color.getRed()) : 0.0;
		const double green = std::isfinite(color.getGreen()) ? std::max(0.0, color.getGreen()) : 0.0;
		const double blue = std::isfinite(color.getBlue()) ? std::max(0.0, color.getBlue()) : 0.0;
		const double maxChannel = std::max(red, std::max(green, blue));

		if (maxChannel <= 0.0 || !std::isfinite(maxChannel))
		{
			throw std::invalid_argument("Spectral color converted outside the visible RGB gamut.");
		}

		return (Color(red / maxChannel, green / maxChannel, blue / maxChannel));
	}

	double	clampUnit(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		if (value >= 1.0)
		{
			return (1.0);
		}
		return (value);
	}

	double	relativePlanckRadiance(double wavelengthNanometers, double temperatureKelvin)
	{
		constexpr double secondRadiationConstant = 1.438776877e-2;
		const double wavelengthMeters = wavelengthNanometers * 1.0e-9;
		const double exponent = secondRadiationConstant / (wavelengthMeters * temperatureKelvin);

		if (exponent > 700.0)
		{
			return (0.0);
		}
		return (1.0 / (std::pow(wavelengthMeters, 5.0) * std::expm1(exponent)));
	}

	double	referenceIlluminant(double wavelengthNanometers)
	{
		return (relativePlanckRadiance(wavelengthNanometers, 6504.0));
	}

	void	validateSpectralSample(const ColorScience::SpectralSample& sample)
	{
		if (
			!std::isfinite(sample.wavelengthNanometers)
			|| sample.wavelengthNanometers < 360.0
			|| sample.wavelengthNanometers > 830.0
		)
		{
			throw std::invalid_argument("Spectral reflectance wavelengths must be finite and within 360-830 nm.");
		}
		if (!std::isfinite(sample.value) || sample.value < 0.0 || sample.value > 1.0)
		{
			throw std::invalid_argument("Spectral reflectance values must be finite and in [0,1].");
		}
	}

	void	sortAndValidateSamples(std::vector<ColorScience::SpectralSample>& samples)
	{
		if (samples.size() < 2)
		{
			throw std::invalid_argument("Spectral reflectance curves require at least two samples.");
		}
		for (const ColorScience::SpectralSample& sample : samples)
		{
			validateSpectralSample(sample);
		}
		std::sort(
			samples.begin(),
			samples.end(),
			[](const ColorScience::SpectralSample& a, const ColorScience::SpectralSample& b)
			{
				return (a.wavelengthNanometers < b.wavelengthNanometers);
			}
		);
		for (std::size_t i = 1; i < samples.size(); i++)
		{
			if (samples[i].wavelengthNanometers <= samples[i - 1].wavelengthNanometers)
			{
				throw std::invalid_argument("Spectral reflectance wavelengths must be unique.");
			}
		}
	}

	double	sampledReflectance(const std::vector<ColorScience::SpectralSample>& samples, double wavelength)
	{
		if (wavelength <= samples.front().wavelengthNanometers)
		{
			return (samples.front().value);
		}
		if (wavelength >= samples.back().wavelengthNanometers)
		{
			return (samples.back().value);
		}

		auto upper = std::upper_bound(
			samples.begin(),
			samples.end(),
			wavelength,
			[](double value, const ColorScience::SpectralSample& sample)
			{
				return (value < sample.wavelengthNanometers);
			}
		);
		const auto lower = upper - 1;
		const double span = upper->wavelengthNanometers - lower->wavelengthNanometers;
		const double t = span > 0.0 ? (wavelength - lower->wavelengthNanometers) / span : 0.0;

		return (lower->value * (1.0 - t) + upper->value * t);
	}

	std::string	trim(const std::string& input)
	{
		std::size_t start = 0;
		std::size_t end = input.size();

		while (start < end && std::isspace(static_cast<unsigned char>(input[start])))
		{
			start++;
		}
		while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1])))
		{
			end--;
		}
		return (input.substr(start, end - start));
	}
}

Color	ColorScience::wavelength(double nanometers)
{
	if (!std::isfinite(nanometers) || nanometers < 360.0 || nanometers > 830.0)
	{
		throw std::invalid_argument("Wavelength must be finite and within 360-830 nm.");
	}

	return (normalizeVisibleColor(xyzToACEScg(
		cieX(nanometers),
		cieY(nanometers),
		cieZ(nanometers)
	)));
}

Color	ColorScience::blackbody(double kelvin)
{
	if (!std::isfinite(kelvin) || kelvin < 1000.0 || kelvin > 40000.0)
	{
		throw std::invalid_argument("Blackbody temperature must be finite and within 1000-40000 K.");
	}

	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	for (double wavelength = 360.0; wavelength <= 830.0; wavelength += 5.0)
	{
		const double radiance = relativePlanckRadiance(wavelength, kelvin);
		x += radiance * cieX(wavelength);
		y += radiance * cieY(wavelength);
		z += radiance * cieZ(wavelength);
	}

	return (normalizeVisibleColor(xyzToACEScg(x, y, z)));
}

Color	ColorScience::solar(void)
{
	return (blackbody(LightUnits::SOLAR_TEMPERATURE_K));
}

Color	ColorScience::reflectanceCurve(std::vector<SpectralSample> samples)
{
	sortAndValidateSamples(samples);

	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	double whiteX = 0.0;
	double whiteY = 0.0;
	double whiteZ = 0.0;
	for (double wavelength = 360.0; wavelength <= 830.0; wavelength += 5.0)
	{
		const double illuminant = referenceIlluminant(wavelength);
		const double reflectance = sampledReflectance(samples, wavelength);
		const double weightedX = illuminant * cieX(wavelength);
		const double weightedY = illuminant * cieY(wavelength);
		const double weightedZ = illuminant * cieZ(wavelength);

		x += reflectance * weightedX;
		y += reflectance * weightedY;
		z += reflectance * weightedZ;
		whiteX += weightedX;
		whiteY += weightedY;
		whiteZ += weightedZ;
	}

	const Color reflected = xyzToACEScg(x, y, z);
	const Color white = xyzToACEScg(whiteX, whiteY, whiteZ);
	if (
		white.getRed() <= 0.0
		|| white.getGreen() <= 0.0
		|| white.getBlue() <= 0.0
		|| !std::isfinite(white.getRed())
		|| !std::isfinite(white.getGreen())
		|| !std::isfinite(white.getBlue())
	)
	{
		throw std::invalid_argument("Spectral reflectance illuminant produced an invalid white point.");
	}

	return (Color(
		clampUnit(reflected.getRed() / white.getRed()),
		clampUnit(reflected.getGreen() / white.getGreen()),
		clampUnit(reflected.getBlue() / white.getBlue())
	));
}

Color	ColorScience::loadReflectanceCurve(const std::string& fileName)
{
	std::ifstream stream(fileName);

	if (!stream)
	{
		throw std::runtime_error("Spectral reflectance curve could not be opened: " + fileName);
	}

	std::vector<SpectralSample> samples;
	std::string line;
	std::size_t lineNumber = 0;
	while (std::getline(stream, line))
	{
		lineNumber++;
		const std::size_t comment = line.find('#');
		if (comment != std::string::npos)
		{
			line = line.substr(0, comment);
		}
		std::replace(line.begin(), line.end(), ',', ' ');
		std::replace(line.begin(), line.end(), ';', ' ');
		line = trim(line);
		if (line.empty())
		{
			continue;
		}

		std::stringstream lineStream(line);
		SpectralSample sample;
		std::string trailing;
		if (!(lineStream >> sample.wavelengthNanometers >> sample.value) || (lineStream >> trailing))
		{
			throw std::runtime_error(
				"Invalid spectral reflectance sample at "
				+ fileName
				+ ":"
				+ std::to_string(lineNumber)
				+ ". Use wavelength_nm,value."
			);
		}
		samples.push_back(sample);
	}

	try
	{
		return (reflectanceCurve(samples));
	}
	catch (const std::exception& exception)
	{
		throw std::runtime_error("Invalid spectral reflectance curve '" + fileName + "': " + exception.what());
	}
}
