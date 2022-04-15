#pragma once

#include "Hittables/Hittable.hpp"
#include "Transform.hpp"

class	Landscape : public Hittable
{
	public:
		Landscape(void);
		Landscape(Transform transform, std::shared_ptr<Material> material, unsigned int seed);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		Transform		_transform;
		std::shared_ptr<Material>	_material;
		unsigned int	_seed;
};