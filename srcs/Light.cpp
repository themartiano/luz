#include "Light.hpp"

/*
	Constructors
*/

// Constructs the Light with default values
Light::Light(void)
{
	this->_color = Color(255, 255, 255, 0);
	this->_transform = Transform();
	this->_brightness = 1.0f;
}

// Constructs the Light with custom values
Light::Light(Color color, Transform transform, float brightness)
{
	this->_color = color;
	this->_transform = transform;
	this->_brightness = brightness;
}
