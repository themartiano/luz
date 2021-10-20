#include "Forms/Sphere.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Sphere with default values
Sphere::Sphere(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(0.49f, 0.49f, 0.49f, 0.0f), 1.0f);
	this->_radius = 1.0f;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Transform transform, Material material, float radius)
{
	this->_transform = transform;
	this->_material = material;
	this->_radius = radius;
}

// Returns the Sphere's material
Material	Sphere::getMaterial(void) const
{
	return (this->_material);
}
// Returns the Sphere's transform
Transform	Sphere::getTransform(void) const
{
	return (this->_transform);
}

// Returns the Sphere's radius
float		Sphere::getRadius(void) const
{
	return (this->_radius);
}
