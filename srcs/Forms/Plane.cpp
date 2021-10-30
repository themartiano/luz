#include "Forms/Plane.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"

/*
	Constructors
*/

// Constructs the Plane with default values
Plane::Plane(void)
{
    this->_y = 0.0f;
    this->_orientation = Vector3(0.0f, -1.0f, 0.0f);
    this->_material = Material(Color(0.49f, 0.49f, 0.49f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f);
}

// Constructs the Plane with custom values
Plane::Plane(float y, Vector3 orientation, Material material)
{
    this->_y = y;
    this->_orientation = orientation;
    this->_material = material;
}

// Calculates if the Plane is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Plane::hit(Ray& ray, float t_max) const
{
    Vector3 position(0.0f, this->_y, 0.0f);

    float d = dot(ray.getDirection(), this->_orientation);
    if (d == 0.0f)
    {
        return (false);
    }

    float t = dot(position - ray.getOrigin(), this->_orientation) / d;
    if (t > t_max || t < T_MIN)
    {
        return (false);
    }

    ray.hitRecord.t = t;
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
