#include "Light.hpp"

/*
	Constructors
*/

// Constructs the Light with default values
Light::Light(void)
{
	this->_color = Color(1.0, 1.0, 1.0);
	this->_transform = Transform();
	this->_brightness = 1.0;
}

// Constructs the Light with custom values
Light::Light(Color color, Transform transform, double brightness)
{
	this->_color = color;
	this->_transform = transform;
	this->_brightness = brightness;
}
