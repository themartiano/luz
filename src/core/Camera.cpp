#include "Camera.hpp"
#include "Defaults.hpp"
#include <cmath>
#include <stdexcept>
#include <string>

namespace
{
	void	requirePositiveFinite(double value, const std::string& name)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			throw std::invalid_argument(name + " must be finite and positive.");
		}
	}
}

/*
	Constructors
*/

// Constructs the Camera with default values
Camera::Camera(void)
{
	this->_position = Vector3(0.0, 0.0, 15.0);
	this->_direction = Vector3(0.0, 0.0, -1.0);
	this->_upDirection = Vector3(0.0, 1.0, 0.0);
	this->_focalLengthMeters = D_CAMERA_FOCAL_LENGTH_METERS;
	this->_sensorWidthMeters = D_CAMERA_SENSOR_WIDTH_METERS;
	this->_sensorHeightMeters = D_CAMERA_SENSOR_HEIGHT_METERS;
	this->_fNumber = D_CAMERA_F_NUMBER;
	this->_pinhole = false;
	this->_focusDistanceMeters = D_CAMERA_FOCUS_DISTANCE_METERS;
}

// Constructs the Camera with custom values
Camera::Camera(Vector3 position, Vector3 direction, double focalLengthMeters, double sensorWidthMeters, double sensorHeightMeters, double fNumber, double focusDistanceMeters)
{
	this->_position = position;
	this->_direction = direction;
	this->_upDirection = Vector3(0.0, 1.0, 0.0);
	this->_focalLengthMeters = D_CAMERA_FOCAL_LENGTH_METERS;
	this->_sensorWidthMeters = D_CAMERA_SENSOR_WIDTH_METERS;
	this->_sensorHeightMeters = D_CAMERA_SENSOR_HEIGHT_METERS;
	this->_fNumber = D_CAMERA_F_NUMBER;
	this->_pinhole = false;
	this->_focusDistanceMeters = D_CAMERA_FOCUS_DISTANCE_METERS;
	this->setFocalLengthMeters(focalLengthMeters);
	this->setSensorWidthMeters(sensorWidthMeters);
	this->setSensorHeightMeters(sensorHeightMeters);
	this->setFNumber(fNumber);
	this->setFocusDistanceMeters(focusDistanceMeters);
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

Vector3	Camera::getUpDirection(void) const
{
	return (this->_upDirection);
}

void	Camera::setUpDirection(Vector3 upDirection)
{
	this->_upDirection = upDirection;
}

double	Camera::getFocalLengthMeters(void) const
{
	return (this->_focalLengthMeters);
}

void	Camera::setFocalLengthMeters(double focalLengthMeters)
{
	requirePositiveFinite(focalLengthMeters, "Camera focal length");
	this->_focalLengthMeters = focalLengthMeters;
}

double	Camera::getSensorWidthMeters(void) const
{
	return (this->_sensorWidthMeters);
}

void	Camera::setSensorWidthMeters(double sensorWidthMeters)
{
	requirePositiveFinite(sensorWidthMeters, "Camera sensor width");
	this->_sensorWidthMeters = sensorWidthMeters;
}

double	Camera::getSensorHeightMeters(void) const
{
	return (this->_sensorHeightMeters);
}

void	Camera::setSensorHeightMeters(double sensorHeightMeters)
{
	requirePositiveFinite(sensorHeightMeters, "Camera sensor height");
	this->_sensorHeightMeters = sensorHeightMeters;
}

double	Camera::getFNumber(void) const
{
	return (this->_fNumber);
}

void	Camera::setFNumber(double fNumber)
{
	requirePositiveFinite(fNumber, "Camera f-number");
	this->_fNumber = fNumber;
	this->_pinhole = false;
}

double	Camera::getApertureDiameterMeters(void) const
{
	if (this->_pinhole)
	{
		return (0.0);
	}
	return (this->_focalLengthMeters / this->_fNumber);
}

void	Camera::setApertureDiameterMeters(double apertureDiameterMeters)
{
	requirePositiveFinite(apertureDiameterMeters, "Camera aperture diameter");
	this->_fNumber = this->_focalLengthMeters / apertureDiameterMeters;
	this->_pinhole = false;
}

bool	Camera::getPinhole(void) const
{
	return (this->_pinhole);
}

void	Camera::setPinhole(bool pinhole)
{
	this->_pinhole = pinhole;
}

double	Camera::getFocusDistanceMeters(void) const
{
	return (this->_focusDistanceMeters);
}

void	Camera::setFocusDistanceMeters(double focusDistanceMeters)
{
	requirePositiveFinite(focusDistanceMeters, "Camera focus distance");
	this->_focusDistanceMeters = focusDistanceMeters;
}
