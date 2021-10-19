#include "Ray.hpp"

Ray::Ray(void)
{
    this->_origin = Vector3(0.0f, 0.0f, 0.0f);
    this->_direction = Vector3(0.0f, 0.0f, 0.0f);
}

Ray::Ray(Vector3 origin, Vector3 direction)
{
	this->_origin = origin;
    direction.setY(-direction.getY());
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

Vector3 Ray::getDirection(void) const
{
    return (this->_direction);
}
