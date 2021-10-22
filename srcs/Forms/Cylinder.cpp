#include "Forms/Cylinder.hpp"

/*
	Constructors
*/

// Constructs the Cylinder with default values
Cylinder::Cylinder(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(0.49f, 0.49f, 0.49f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f);
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
