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

bool	Sphere::hit(Ray ray)
{
	Vector3 oc = ray.getOrigin() - this->_transform.getPosition();
	float a = dot(ray.getDirection(), ray.getDirection());
	float b = 2.0f * dot(oc, ray.getDirection());
	float c = dot(oc, oc) - this->_radius * this->_radius;
	float discriminant = b * b - 4.0f * a * c;
	return (discriminant > 0.0f);
}

Material	Sphere::getMaterial(void) const
{
	return (this->_material);
}
