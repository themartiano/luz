#include "ColorScience.hpp"
#include "ColorManagement.hpp"
#include "LightUnits.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

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
