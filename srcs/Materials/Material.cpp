#include "Materials/Material.hpp"

/*
	Constructors
*/

// Constructs the Material with default values
Material::Material(void)
{
	this->_color = Color(0.0, 0.0, 0.0);
}

// Constructs the Material with custom values
Material::Material(Color color)
{
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

bool	Material::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	return (false);
	(void)ray;
	(void)hitRecord;
	(void)scatterRecord;
}

Color	Material::emitted(void)
{
	return (Color(0.0, 0.0, 0.0));
}

double	Material::scatteringPDF(Ray& ray, HitRecord& hitRecord)
{
	return (0.0);
	(void)ray;
	(void)hitRecord;
}

MaterialType	Material::getType(void) const
{
	return (BASIC);
}
