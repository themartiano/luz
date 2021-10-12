#include "Sphere.hpp"

/*
	Constructors
*/

Sphere::Sphere(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(126, 126, 126, 0), 1.0f);
	this->_radius = 1.0f;
}

Sphere::Sphere(Transform transform, Material material, float radius)
{
	this->_transform = transform;
	this->_material = material;
	this->_radius = radius;
}
