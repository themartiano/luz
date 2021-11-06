#include "Camera.hpp"

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_position = Vector3(0.0, 0.0, 15.0);
	this->_direction = Vector3(0.0, 0.0, 0.0);
	this->_fov = 65;
	this->_aperture = 2.0;
}

// Constructs the Camera with custom values
Camera::Camera(Vector3 position, Vector3 direction, short fov, double aperture)
{
	this->_position = position;
	this->_direction = direction;
	this->_fov = fov;
	this->_aperture = aperture;
}

// Returns the Camera's position
Vector3		Camera::getPosition(void) const
{
	return (this->_position);
}

// Returns the Camera's direction (facing direction, look direction)
Vector3		Camera::getDirection(void) const
{
	return (this->_direction);
}

// Returns the Camera's FOV (Field of View)
short		Camera::getFOV(void) const
{
	return (this->_fov);
}

// Returns the Camera's Aperture
double	Camera::getAperture(void) const
{
	return (this->_aperture);
}
