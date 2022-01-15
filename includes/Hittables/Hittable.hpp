#pragma once

#include "AABB.hpp"
#include "Materials/Material.hpp"
#include "Ray/Ray.hpp"
#include "Vector3.hpp"

class	AABB;
class	Material;

struct	HitRecord
{
	double		t0 = 0.0;
	double		t1 = 0.0;
	Vector3		position;
	Vector3		normal;
	std::shared_ptr<Material>	material = nullptr;
};

class   Hittable
{
	public:
		virtual ~Hittable(void) = default;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const = 0;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const = 0;
		virtual std::shared_ptr<Material>	getMaterial(void) const = 0;
		virtual double pdfValue(const Vector3& origin, const Vector3& vec) const;
		virtual Vector3 random(const Vector3& origin) const;
};
