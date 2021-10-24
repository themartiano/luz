#include "Material.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Material with default values
Material::Material(void)
{
	this->_color = Color(0.0f, 0.0f, 0.0f);
	this->_metallic = 0.0f;
	this->_albedo = 0.5f;
	this->_opacity = 1.0f;
}

// Constructs the Material with custom values
Material::Material(Color color, float opacity, float metallic, float albedo, float reflectionFuzziness, bool isDielectric)
{
	setFloatRange(metallic, 0.0f, 1.0f);
	this->_metallic = metallic;
	setFloatRange(albedo, 0.0f, 1.0f);
	this->_albedo = albedo;
	setFloatRange(opacity, 0.0f, 1.0f);
	this->_opacity = opacity;
	setFloatRange(reflectionFuzziness, 0.0f, 1.0f);
	this->_reflectionFuzziness = reflectionFuzziness;
	this->_isDielectric = isDielectric;
	this->_color = color;
}

// Returns the Material's color
Color	Material::getColor(void) const
{
	return (this->_color);
}

// Sets the Material's color
void	Material::setColor(Color color)
{
	this->_color = color;
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

// Returns the Material's reflection fuzziness
float	Material::getReflectionFuzziness(void) const
{
	return (this->_reflectionFuzziness);
}

// Returns the Material's dielectric value
bool	Material::getIsDielectric(void) const
{
	return (this->_isDielectric);
}
