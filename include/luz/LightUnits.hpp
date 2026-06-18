#pragma once

#include "Color.hpp"

namespace LightUnits
{
	constexpr double	LUMENS_PER_RADIANT_WATT = 683.0;

	Color	surfaceRadiance(Color color, double radiance);
	Color	surfaceLuminance(Color color, double luminance);
	Color	surfaceRadiantPower(Color color, double watts, double area);
	Color	surfaceLuminousFlux(Color color, double lumens, double area);
	Color	directionalIrradiance(Color color, double irradiance);
	Color	directionalIlluminance(Color color, double illuminance);
}
