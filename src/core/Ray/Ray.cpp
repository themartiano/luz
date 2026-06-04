#include "Ray/Ray.hpp"
#include "Scene/Scene.hpp"
#include "Utilities.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Ray with default values
Ray::Ray(void)
{
	this->_origin = Vector3(0.0, 0.0, 0.0);
	this->_direction = Vector3(0.0, 0.0, 0.0);
}

// Constructs the Ray with custom values
Ray::Ray(Vector3 origin, Vector3 direction)
{
	this->_origin = origin;
	this->_direction = Utilities::normalize(direction);
}

// Returns a point on the ray where 't' determines its position
Vector3 Ray::pointAtRay(double t)
{
	return (this->_origin + (this->_direction * t));
}

// Returns the Ray's origin
Vector3 Ray::getOrigin(void) const
{
	return (this->_origin);
}

// Sets the Ray's origin
void Ray::setOrigin(Vector3 origin)
{
	this->_origin = origin;
}

// Returns the Ray's direction
Vector3 Ray::getDirection(void) const
{
	return (this->_direction);
}

// Sets the Ray's direction
void Ray::setDirection(Vector3 direction)
{
	this->_direction = Utilities::normalize(direction);
}
