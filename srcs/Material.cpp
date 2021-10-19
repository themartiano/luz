#include "Material.hpp"

/*
	Constructors
*/

// Constructs the Material with default values
Material::Material(void)
{
	this->_color = Color(0.0f, 0.0f, 0.0f, 0.0f);
	this->_opacity = 1.0f;
}

// Constructs the Material with custom values
Material::Material(Color color, float opacity)
{
	this->_color = color;
	this->_opacity = opacity;
}

// Returns the Material's color
Color	Material::getColor(void) const
{
	return (this->_color);
}
