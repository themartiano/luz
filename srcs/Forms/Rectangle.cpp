#include "Forms/Rectangle.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Rectangle with default values
Rectangle::Rectangle(void)
{
    this->_transform = Transform(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));
    this->_sideSize = 0.0f;
	this->_material = Material(Color(0.49f, 0.49f, 0.49f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f);
}

// Constructs the Rectangle with custom values
Rectangle::Rectangle(Transform transform, float sideSize, Material material)
{
    this->_transform = transform;
    this->_sideSize = sideSize;
    this->_material = material;
}

// Returns the Rectangle's transform
Transform Rectangle::getTransform(void) const
{
    return (this->_transform);
}

// Returns the Rectangle's side size
float Rectangle::getSideSize(void) const
{
    return (this->_sideSize);
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Rectangle::hit(Ray& ray, float t_max) const
{
    float   halfSideSize = this->_sideSize / 2.0f;

    float a = dot(ray.getOrigin() - this->_transform.getPosition(), this->_transform.getOrientation());
    float b = dot(ray.getDirection(), this->_transform.getOrientation());
    if (b == 0.0f || (a < 0.0f && b < 0.0f) || (a > 0.0f && b > 0.0f))
    {
        return (false);
    }

    float   t = -a / b;
    Vector3 d = (ray.getOrigin() + ray.getDirection() * t) - this->_transform.getPosition();
    if (fabs(d.getX()) > halfSideSize || fabs(d.getY()) > halfSideSize || fabs(d.getZ()) > halfSideSize
        || t > t_max || t < T_MIN)
    {
        return (false);
    }

    ray.hitRecord.t = t;
    ray.hitRecord.normal = this->_transform.getOrientation();
    ray.hitRecord.material = this->_material;
    ray.hitRecord.position = ray.pointAtRay(t);

    return (true);
}

// Creates an AABB / bounding box for this Rectangle
bool    Rectangle::createBoundingBox(AABB& outputBoundingBox) const
{
	// outputBoundingBox = AABB(
    //     Vector3(this->_x0, this->_y0, this->_position.getZ() - T_MIN),
    //     Vector3(this->_x1, this->_y1, this->_position.getZ() + T_MIN));

    return (true);
}