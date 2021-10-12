#include "Cylinder.hpp"

/*
	Constructors
*/

// Constructs the Cylinder with default values
Cylinder::Cylinder(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(126, 126, 126, 0), 1.0f);
	this->_radius = 1.0f;
	this->_height = 3.0f;
}

// Constructs the Cylinder with custom values
Cylinder::Cylinder(Transform transform, Material material, float radius, float height)
{
	this->_transform = transform;
	this->_material = material;
	this->_radius = radius;
	this->_height = height;
}
