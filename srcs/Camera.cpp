#include "Camera.hpp"

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_transform = Transform();
	this->_fov = 65;
	this->_aperture = 2.0f;
}

// Constructs the Camera with custom values
Camera::Camera(Transform transform, short fov, float aperture)
{
	this->_transform = transform;
	this->_fov = fov;
	this->_aperture = aperture;
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

// Returns the Camera's Aperture
float	Camera::getAperture(void) const
{
	return (this->_aperture);
}
