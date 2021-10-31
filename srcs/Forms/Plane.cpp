#include "Forms/Plane.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"

/*
	Constructors
*/

// Constructs the Plane with default values
Plane::Plane(void)
{
    this->_y = 0.0;
    this->_orientation = Vector3(0.0, -1.0, 0.0);
    this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
}

// Constructs the Plane with custom values
Plane::Plane(double y, Vector3 orientation, Material material)
{
    this->_y = y;
    this->_orientation = orientation;
    this->_material = material;
}

// Calculates if the Plane is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Plane::hit(Ray& ray, double t_max) const
{
    Vector3 position(0.0, this->_y, 0.0);

    double d = dot(ray.getDirection(), this->_orientation);
    if (d == 0.0)
    {
        return (false);
    }

    double t = dot(position - ray.getOrigin(), this->_orientation) / d;
    if (t > t_max || t < T_MIN)
    {
        return (false);
    }

    ray.hitRecord.t0 = t;
    ray.hitRecord.normal = this->_orientation;
    ray.hitRecord.material = this->_material;
    ray.hitRecord.position = ray.pointAtRay(t);

    return (true);
}

// Won't create an AABB / bounding box because planes can't have them since they're infinite. Returns false
bool    Plane::createBoundingBox(AABB& boundingBox) const
{
    return (false);
}
