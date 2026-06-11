#pragma once

#include "Hittables/Hittable.hpp"
#include "Vector3.hpp"
#include "Ray/Ray.hpp"

struct	HitRecord;

class   AABB
{
	public:
		AABB(void);
		AABB(Vector3 minimum, Vector3 maximum);
		const Vector3&	getMinimum(void) const;
		const Vector3&	getMaximum(void) const;
		bool	hit(Ray& ray, HitRecord& hitRecord, double t_max) const;

	private:
		Vector3 _minimum;
		Vector3 _maximum;
};
