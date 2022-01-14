#include "Materials/Emissive.hpp"

Color	Emissive::emitted(Ray& ray)
{
	return (this->_color * this->_intensity);
}

void	Emissive::setIntensity(double newIntensity)
{
	this->_intensity = newIntensity;
}
