#include "Camera.hpp"

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_transform = Transform();
	this->_fov = 65;
}

// Constructs the Camera with custom values
Camera::Camera(Transform transform, short fov)
{
	this->_transform = transform;
	this->_fov = fov;
}

// Returns the Camera's transform
Transform	Camera::getTransform(void) const
{
	return (this->_transform);
}

// Returns the Camera's FOV (Field of View)
short		Camera::getFOV(void) const
{
	return (this->_fov);
}
