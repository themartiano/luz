#include "Material.hpp"

/*
	Constructors
*/

Material::Material(void)
{
	this->_color = Color(0, 0, 0, 0);
	this->_opacity = 1.0f;
}

Material::Material(Color color, float opacity)
{
	this->_color = color;
	this->_opacity = opacity;
}
