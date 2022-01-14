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

bool	scatter(Ray& ray)
{
	return (false);
}
