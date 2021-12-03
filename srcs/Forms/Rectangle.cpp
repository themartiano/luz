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
    this->_transform = Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0));
	this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
    this->_width = 1.0;
    this->_height = 1.0;
}

// Constructs the Rectangle with custom values
Rectangle::Rectangle(Transform transform, Material material, double width, double height)
{
    this->_transform = transform;
    this->_material = material;
    this->_width = width;
    this->_height = height;
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Rectangle::hit(Ray& ray, double t_max) const
{
    double a = Utilities::dot(ray.getOrigin() - this->_transform.getPosition(), this->_transform.getOrientation());
    double b = Utilities::dot(ray.getDirection(), this->_transform.getOrientation());
    if (b == 0.0 || (a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0))
    {
        return (false);
    }

    double t = -a / b;
    if (t > t_max || t < T_MIN)
    {
        return (false);
    }

    Vector3 d = ray.pointAtRay(t) - this->_transform.getPosition();
    if (fabs(this->_transform.getOrientation().getY()) > 0.0)
    {
        if (fabs(d.getX()) > (this->_width / 2.0) || fabs(d.getZ()) > (this->_height / 2.0))
        {
            return (false);
        }
    }
    else if (fabs(this->_transform.getOrientation().getZ()) > 0.0)
    {
        if (fabs(d.getX()) > (this->_width / 2.0) || fabs(d.getY()) > (this->_height / 2.0))
        {
            return (false);
        }
    }
    else if (fabs(this->_transform.getOrientation().getX()) > 0.0)
    {
        if (fabs(d.getZ()) > (this->_width / 2.0) || fabs(d.getY()) > (this->_height / 2.0))
        {
            return (false);
        }
    }

    ray.hitRecord.t0 = t;
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
    (void)outputBoundingBox;
}
