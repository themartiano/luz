#include "Materials/Emissive.hpp"

Emissive::Emissive(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
}

Emissive::Emissive(Color color)
{
	this->_color = color;
}

Emissive::Emissive(double intensity)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_intensity = intensity;
}

Emissive::Emissive(Color color, double intensity)
{
	this->_color = color;
	this->_intensity = intensity;
}

Color	Emissive::emitted(Ray& ray)
{
	return (this->_color * this->_intensity);
}

void	Emissive::setIntensity(double newIntensity)
{
	this->_intensity = newIntensity;
}
