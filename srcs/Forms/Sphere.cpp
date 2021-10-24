#include "Forms/Sphere.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Sphere with default values
Sphere::Sphere(void)
{
	this->_position = Vector3();
	this->_material = Material(Color(0.49f, 0.49f, 0.49f), 1.0f, 0.0f, 0.5f, 0.0f, false);
	this->_radius = 1.0f;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Vector3 position, Material material, float radius)
{
	this->_position = position;
	this->_material = material;
	this->_radius = radius;
}

// Returns the Sphere's position
Vector3	Sphere::getPosition(void) const
{
	return (this->_position);
}

// Returns the Sphere's material
Material	Sphere::getMaterial(void) const
{
	return (this->_material);
}

// Returns the Sphere's radius
float		Sphere::getRadius(void) const
{
	return (this->_radius);
}
