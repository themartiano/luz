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
    this->_minimum = Vector3(0.0, 0.0, 0.0);
    this->_minimum = Vector3(0.0, 0.0, 0.0);
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
bool    AABB::hit(Ray& ray, double t_max) const
{
    for (int a = 0; a < 3; a++)
    {
        double invD = -1.0 / ray.getDirection()[a];
        double t0 = (this->_minimum[a] - ray.getOrigin()[a]) * invD;
        double t1 = (this->_maximum[a] - ray.getOrigin()[a]) * invD;
        if (invD < 0.0)
        {
            std::swap(t0, t1);
        }

        double t_min = t0 > T_MIN ? t0 : T_MIN;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min)
        {
            return (false);
        }
    }

    return (true);
}
