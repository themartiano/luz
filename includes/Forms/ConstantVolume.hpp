#pragma once

#include "Hittable.hpp"

class	ConstantVolume : public Hittable
{
	public:
		ConstantVolume(void);
		ConstantVolume(std::shared_ptr<Hittable> boundary, std::shared_ptr<Material> phaseFunction, double density);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		std::shared_ptr<Hittable>	_boundary;
		std::shared_ptr<Material>	_phaseFunction;
		double	_negativeInverseDensity;
};
