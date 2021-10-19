#include "Transform.hpp"

/*
	Constructors
*/

// Constructs the Transform with default values
Transform::Transform(void)
{
	this->_position = Vector3(0.0f, 0.0f, 0.0f);
	this->_orientation = Vector3(0.0f, 0.0f, 0.0f);
	this->_scale = Vector3(1.0f, 1.0f, 1.0f);
}

// Constructs the Transform with custom values
Transform::Transform(Vector3 position, Vector3 orientation, Vector3 scale)
{
	this->_position = position;
	this->_orientation = orientation;
	this->_scale = scale;
}

// Returns the Transform's position value
Vector3	Transform::getPosition(void) const
{
	return (this->_position);
}

// Returns the Transform's orientation value
Vector3	Transform::getOrientation(void) const
{
	return (this->_orientation);
}

// Returns the Transform's scale value
Vector3	Transform::getScale(void) const
{
	return (this->_scale);
}
