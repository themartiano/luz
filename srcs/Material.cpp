#include "Material.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Material with default values
Material::Material(void)
{
	this->_color = Color(0.0f, 0.0f, 0.0f, 0.0f);
	this->_metallic = 0.0f;
	this->_albedo = 0.5f;
	this->_opacity = 1.0f;
}

// Constructs the Material with custom values
Material::Material(Color color, float opacity, float metallic, float albedo)
{
	setFloatRange(metallic, 0.0f, 1.0f);
	this->_metallic = metallic;
	setFloatRange(albedo, 0.0f, 1.0f);
	this->_albedo = albedo;
	this->_color = color;
	setFloatRange(opacity, 0.0f, 1.0f);
	this->_opacity = opacity;
}

// Returns the Material's color
Color	Material::getColor(void) const
{
	return (this->_color);
}

// Returns the Material's metallic value
float	Material::getMetallic(void) const
{
	return (this->_metallic);
}

// Returns the Material's albedo
float	Material::getAlbedo(void) const
{
	return (this->_albedo);
}
