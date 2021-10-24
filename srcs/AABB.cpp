#include "AABB.hpp"
#include "Defaults.hpp"
#include <string_view>
#include <cmath>

/*
	Constructors
*/

// Constructs the AABB with default values
AABB::AABB(void)
{
    this->_minimum = Vector3(0.0f, 0.0f, 0.0f);
    this->_minimum = Vector3(0.0f, 0.0f, 0.0f);
}

// Constructs the AABB with custom values
AABB::AABB(Vector3 minimum, Vector3 maximum)
{
    this->_minimum = minimum;
    this->_maximum = maximum;
}

// Returns the AABB's minimum value
Vector3 AABB::getMinimum(void) const
{
    return (this->_minimum);
}

// Returns the AABB's maximum value
Vector3 AABB::getMaximum(void) const
{
    return (this->_maximum);
}

// Returns true if 'ray' intersects with the AABB
bool    AABB::hit(Ray ray, float t_max) const
{
    for (int a = 0; a < 3; a++)
    {
        // float invD = 1.0f / ray.getDirection()[a];
        // float t0 = (this->_minimum[a] - ray.getOrigin()[a]) * invD;
        // float t1 = (this->_maximum[a] - ray.getOrigin()[a]) * invD;
        // if (invD < 0.0f)
        // {
        //     std::swap(t0, t1);
        // }

        // float t_min = t0 > T_MIN ? t0 : T_MIN;
        // t_max = t1 < t_max ? t1 : t_max;
        // if (t_max <= t_min)
        // {
        //     return (false);
        // }
        auto t0 = fmin((this->_minimum[a] - ray.getOrigin()[a]) / ray.getDirection()[a], (this->_maximum[a] - ray.getOrigin()[a]) / ray.getDirection()[a]);
        auto t1 = fmax((this->_minimum[a] - ray.getOrigin()[a]) / ray.getDirection()[a], (this->_maximum[a] - ray.getOrigin()[a]) / ray.getDirection()[a]);

        float t_min = fmax(t0, T_MIN);
        t_max = fmin(t1, t_max);
        if (t_max <= t_min)
        {
            return (false);
        }
    }

    return (true);
}
