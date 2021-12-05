#include "Camera.hpp"

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_position = Vector3(0.0, 0.0, 15.0);
	this->_direction = Vector3(0.0, 0.0, -1.0);
	this->_fov = 65;
	this->_aperture = 2.0;
	this->_focusDistance = 10.0;
}

// Constructs the Camera with custom values
Camera::Camera(Vector3 position, Vector3 direction, short fov, double aperture, double focusDistance)
{
	this->_position = position;
	this->_direction = direction;
	this->_fov = fov;
	this->_aperture = aperture;
	this->_focusDistance = focusDistance;
}

// Returns the Camera's position
Vector3		Camera::getPosition(void) const
{
	return (this->_position);
}

// Sets the Camera's position
void	Camera::setPosition(Vector3 position)
{
	this->_position = position;
}

// Returns the Camera's direction (facing direction, look direction)
Vector3		Camera::getDirection(void) const
{
	return (this->_direction);
}

// Sets the Camera's direction
void	Camera::setDirection(Vector3 direction)
{
	this->_direction = direction;
}

// Returns the Camera's FOV (Field of View)
short		Camera::getFOV(void) const
{
	return (this->_fov);
}

// Sets the Camera's FOV
void	Camera::setFOV(short fov)
{
	this->_fov = fov;
}

// Returns the Camera's Aperture
double	Camera::getAperture(void) const
{
	return (this->_aperture);
}

// Sets the Camera's aperture
void	Camera::setAperture(double aperture)
{
	this->_aperture = aperture;
}

// Returns the Camera's Focus Distance (used for Depth of Field / DOF)
double	Camera::getFocusDistance(void) const
{
	return (this->_focusDistance);
}

// Sets the Camera's Focus Distance
void	Camera::setFocusDistance(double focusDistance)
{
	this->_focusDistance = focusDistance;
}
