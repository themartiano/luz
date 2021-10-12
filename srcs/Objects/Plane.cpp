#include "Plane.hpp"

/*
	Constructors
*/

Plane::Plane(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(126, 126, 126, 0), 1.0f);
}

Plane::Plane(Transform transform, Material material)
{
	this->_transform = transform;
	this->_material = material;
}
