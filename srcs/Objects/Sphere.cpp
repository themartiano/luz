#include "Objects/Sphere.hpp"

/*
	Constructors
*/

// Constructs the Sphere with default values
Sphere::Sphere(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(126, 126, 126, 0), 1.0f);
	this->_radius = 1.0f;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Transform transform, Material material, float radius)
{
	this->_transform = transform;
	this->_material = material;
	this->_radius = radius;
}

Material	Sphere::getMaterial(void) const
{
	return (this->_material);
}

Transform	Sphere::getTransform(void) const
{
	return (this->_transform);
}

float		Sphere::getRadius(void) const
{
	return (this->_radius);
}
