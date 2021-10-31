#include "Material.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Material with default values
Material::Material(void)
{
	this->_color = Color(0.0, 0.0, 0.0);
	this->_metallic = 0.0;
	this->_albedo = 0.5;
	this->_opacity = 1.0;
	this->_reflectionFuzziness = 0.0;
	this->_isDielectric = false;
	this->_isEmissive = false;
	this->_lightIntensity = 0.0;
}

// Constructs the Material with custom values
Material::Material(Color color, double opacity, double metallic, double albedo, double reflectionFuzziness, bool isDielectric, bool isEmissive, double lightIntensity)
{
	setdoubleRange(metallic, 0.0, 1.0);
	this->_metallic = metallic;
	setdoubleRange(albedo, 0.0, 1.0);
	this->_albedo = albedo;
	setdoubleRange(opacity, 0.0, 1.0);
	this->_opacity = opacity;
	setdoubleRange(reflectionFuzziness, 0.0, 1.0);
	this->_reflectionFuzziness = reflectionFuzziness;
	this->_isDielectric = isDielectric;
	this->_isEmissive = isEmissive;
	this->_color = color;
	this->_lightIntensity = lightIntensity;
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
double	Material::getMetallic(void) const
{
	return (this->_metallic);
}

// Returns the Material's albedo
double	Material::getAlbedo(void) const
{
	return (this->_albedo);
}

// Returns the Material's reflection fuzziness
double	Material::getReflectionFuzziness(void) const
{
	return (this->_reflectionFuzziness);
}

// Returns the Material's dielectric value
bool	Material::getIsDielectric(void) const
{
	return (this->_isDielectric);
}

// Returns the Material's emissive value
bool	Material::getIsEmissive(void) const
{
	return (this->_isEmissive);
}

// Returns the Material's light intensity
double	Material::getLightIntensity(void) const
{
	return (this->_lightIntensity);
}
