#include "Transform.hpp"

/*
	Constructors
*/

Transform::Transform(void)
{
	this->_position = Vector3(0.0f, 0.0f, 0.0f);
	this->_orientation = Vector3(0.0f, 0.0f, 0.0f);
	this->_scale = Vector3(1.0f, 1.0f, 1.0f);
}

Transform::Transform(Vector3 position, Vector3 orientation, Vector3 scale)
{
	this->_position = position;
	this->_orientation = orientation;
	this->_scale = scale;
}
