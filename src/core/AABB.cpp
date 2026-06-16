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
	this->_maximum = Vector3(0.0, 0.0, 0.0);
}

// Constructs the AABB with custom values
AABB::AABB(Vector3 minimum, Vector3 maximum)
{
	this->_minimum = minimum;
	this->_maximum = maximum;
}

// Returns the AABB's minimum value
const Vector3& AABB::getMinimum(void) const
{
	return (this->_minimum);
}

// Returns the AABB's maximum value
const Vector3& AABB::getMaximum(void) const
{
	return (this->_maximum);
}

// Returns true if 'ray' intersects with the AABB
bool	AABB::hit(Ray& ray, HitRecord& hitRecord, double t_max) const
{
	const Vector3& origin = ray.getOrigin();
	const Vector3& direction = ray.getDirection();
	const Vector3& inverseDirection = ray.getInverseDirection();
	double t_min = T_MIN;

	if (direction[0] == 0.0 && direction[1] == 0.0 && direction[2] == 0.0)
	{
		return (false);
	}
	for (int a = 0; a < 3; a++)
	{
		double invD = inverseDirection[a];
		if (invD == 0.0)
		{
			if (origin[a] < this->_minimum[a] || origin[a] > this->_maximum[a])
			{
				return (false);
			}
			continue;
		}
		double t0 = (this->_minimum[a] - origin[a]) * invD;
		double t1 = (this->_maximum[a] - origin[a]) * invD;
		if (invD < 0.0)
		{
			std::swap(t0, t1);
		}

		t_min = t0 > t_min ? t0 : t_min;
		t_max = t1 < t_max ? t1 : t_max;
		if (t_max <= t_min)
		{
			return (false);
		}
	}

	if (RENDER_AABB)
	{
		hitRecord.t0 = t_max;
	}

	return (true);
}
