#include "Camera.hpp"

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_transform = Transform();
	this->_fov = 70;
}

// Constructs the Camera with custom values
Camera::Camera(Transform transform, short fov)
{
	this->_transform = transform;
	this->_fov = fov;
}

Transform	Camera::getTransform(void) const
{
	return (this->_transform);
}
