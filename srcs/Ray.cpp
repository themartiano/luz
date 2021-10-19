#include "Ray.hpp"
#include "Scene.hpp"
#include <cmath>

Ray::Ray(void)
{
    this->_origin = Vector3(0.0f, 0.0f, 0.0f);
    this->_direction = Vector3(0.0f, 0.0f, 0.0f);
}

Ray::Ray(Vector3 origin, Vector3 direction)
{
	this->_origin = origin;
    this->_direction = direction;
}

// Returns a point on the ray where 't' determines the position
Vector3 Ray::pointAtRay(float t)
{
    return (this->_origin + (Vector3)(this->_direction * t));
}

Vector3 Ray::getOrigin(void) const
{
    return (this->_origin);
}

void Ray::setOrigin(Vector3 origin)
{
	this->_origin = origin;
}

Vector3 Ray::getDirection(void) const
{
    return (this->_direction);
}

void Ray::setDirection(Vector3 direction)
{
    this->_direction = direction;
}
