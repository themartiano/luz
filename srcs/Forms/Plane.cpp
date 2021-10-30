#include "Forms/Plane.hpp"

/*
	Constructors
*/

// Constructs the Plane with default values
Plane::Plane(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(0.49f, 0.49f, 0.49f), 1.0f, 0.0f, 0.5f, 0.0f, false, false);
}

// Constructs the Plane with custom values
Plane::Plane(Transform transform, Material material)
{
	this->_transform = transform;
	this->_material = material;
}
