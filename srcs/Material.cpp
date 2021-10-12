#include "Material.hpp"

/*
	Constructors
*/

// Constructs the Material with default values
Material::Material(void)
{
	this->_color = Color(0, 0, 0, 0);
	this->_opacity = 1.0f;
}

// Constructs the Material with custom values
Material::Material(Color color, float opacity)
{
	this->_color = color;
	this->_opacity = opacity;
}
