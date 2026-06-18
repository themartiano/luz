#include "LightUnits.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <stdexcept>
#include <string>

namespace
{
	void	requireFiniteNonNegative(double value, const std::string& description)
	{
		if (!std::isfinite(value) || value < 0.0)
		{
			throw std::invalid_argument(description + " must be finite and non-negative.");
		}
	}

	void	requirePositive(double value, const std::string& description)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			throw std::invalid_argument(description + " must be finite and positive.");
		}
	}

	void	requireFiniteNonNegativeColor(const Color& color)
	{
		requireFiniteNonNegative(color.getRed(), "Light color red channel");
		requireFiniteNonNegative(color.getGreen(), "Light color green channel");
		requireFiniteNonNegative(color.getBlue(), "Light color blue channel");
	}

	Color	scaleToLuminance(Color color, double targetLuminance)
	{
		requireFiniteNonNegativeColor(color);
		requireFiniteNonNegative(targetLuminance, "Target luminance");
		if (targetLuminance == 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const double colorLuminance = Utilities::luminance(color);
		if (!std::isfinite(colorLuminance) || colorLuminance <= 0.0)
		{
			throw std::invalid_argument("Light color must have positive luminance for physical units.");
		}

		return (color * (targetLuminance / colorLuminance));
	}
}

Color	LightUnits::surfaceRadiance(Color color, double radiance)
{
	requireFiniteNonNegative(radiance, "Surface radiance");
	return (scaleToLuminance(color, radiance));
}

Color	LightUnits::surfaceLuminance(Color color, double luminance)
{
	requireFiniteNonNegative(luminance, "Surface luminance");
	return (surfaceRadiance(color, luminance / LUMENS_PER_RADIANT_WATT));
}

Color	LightUnits::surfaceRadiantPower(Color color, double watts, double area)
{
	requireFiniteNonNegative(watts, "Radiant power");
	requirePositive(area, "Emitter area");
	return (surfaceRadiance(color, watts / (D_PI * area)));
}

Color	LightUnits::surfaceLuminousFlux(Color color, double lumens, double area)
{
	requireFiniteNonNegative(lumens, "Luminous flux");
	requirePositive(area, "Emitter area");
	return (surfaceLuminance(color, lumens / (D_PI * area)));
}

Color	LightUnits::directionalIrradiance(Color color, double irradiance)
{
	requireFiniteNonNegative(irradiance, "Directional irradiance");
	return (scaleToLuminance(color, irradiance));
}

Color	LightUnits::directionalIlluminance(Color color, double illuminance)
{
	requireFiniteNonNegative(illuminance, "Directional illuminance");
	return (directionalIrradiance(color, illuminance / LUMENS_PER_RADIANT_WATT));
}
