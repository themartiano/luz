#pragma once

#include "Color.hpp"

namespace LightUnits
{
	constexpr double	LUMENS_PER_RADIANT_WATT = 683.0;
	constexpr double	SOLAR_TEMPERATURE_K = 5778.0;
	constexpr double	SOLAR_DIRECT_IRRADIANCE_W_M2 = 1361.0;
	constexpr double	SOLAR_DIRECT_ILLUMINANCE_LUX = 110000.0;
	constexpr double	SOLAR_ANGULAR_DIAMETER_DEGREES = 0.533;

	double	solarSolidAngle(void);
	Color	surfaceRadiance(Color color, double radiance);
	Color	surfaceLuminance(Color color, double luminance);
	Color	surfaceRadiantPower(Color color, double watts, double area);
	Color	surfaceLuminousFlux(Color color, double lumens, double area);
	Color	sphericalRadiantIntensity(Color color, double wattsPerSteradian, double surfaceArea);
	Color	sphericalLuminousIntensity(Color color, double candela, double surfaceArea);
	Color	directionalIrradiance(Color color, double irradiance);
	Color	directionalIlluminance(Color color, double illuminance);
	Color	solarDirectionalIrradiance(Color color, double scale);
	Color	solarDiskRadiance(Color color, double scale);
}
