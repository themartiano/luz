#pragma once

#include "Hittables/Hittable.hpp"
#include "Noise/Perlin.hpp"

class	Landscape : public Hittable
{
	public:
		Landscape(void);
		Landscape(std::shared_ptr<Material> material, double noiseScale, unsigned int seed, unsigned int samplesPerRay);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		double	_getHeightAtPoint(double x, double z) const;

		std::shared_ptr<Material>	_material;
		double			_noiseScale; // The scale of the Perlin noise
		unsigned int	_seed;
		unsigned int	_samplesPerRay;

		Perlin			_perlin;
};
