#include "Materials/Emissive.hpp"
#include "LightUnits.hpp"

Emissive::Emissive(void)
{
	this->_color = Color(1.0, 1.0, 1.0);
}

Emissive::Emissive(Color radiance)
{
	this->_color = radiance;
}

Emissive	Emissive::fromRadiance(Color color, double radiance)
{
	return (Emissive(LightUnits::surfaceRadiance(color, radiance)));
}

Emissive	Emissive::fromLuminance(Color color, double luminance)
{
	return (Emissive(LightUnits::surfaceLuminance(color, luminance)));
}

Emissive	Emissive::fromRadiantPower(Color color, double watts, double area)
{
	return (Emissive(LightUnits::surfaceRadiantPower(color, watts, area)));
}

Emissive	Emissive::fromLuminousFlux(Color color, double lumens, double area)
{
	return (Emissive(LightUnits::surfaceLuminousFlux(color, lumens, area)));
}

Color	Emissive::emitted(void)
{
	return (this->_color);
}

void	Emissive::setRadiance(Color radiance)
{
	this->_color = radiance;
}

MaterialType	Emissive::getType(void) const
{
	return (EMISSIVE);
}
