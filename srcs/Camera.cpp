#include "Camera.hpp"

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_lookFrom = Vector3(0.0f, 0.0f, 15.0f);
	this->_lookAt = Vector3(0.0f, 0.0f, 0.0f);
	this->_fov = 65;
	this->_aperture = 2.0f;
}

// Constructs the Camera with custom values
Camera::Camera(Vector3 lookFrom, Vector3 lookAt, short fov, float aperture)
{
	this->_lookFrom = lookFrom;
	this->_lookAt = lookAt;
	this->_fov = fov;
	this->_aperture = aperture;
}

Vector3		Camera::getLookFrom(void) const
{
	return (this->_lookFrom);
}

Vector3		Camera::getLookAt(void) const
{
	return (this->_lookAt);
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
