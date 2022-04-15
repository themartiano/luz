#pragma once

#include "Hittables/Hittable.hpp"
#include "Noise/Perlin.hpp"

class	Landscape : public Hittable
{
	public:
		Landscape(void);
		Landscape(Vector3 position, double size, std::shared_ptr<Material> material, unsigned int subSamples, double noiseScale, unsigned int seed);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		double	_getHeightAtPoint(double x, double z) const;
		Vector3	_getNormalAtPosition(Vector3 position, double t_min) const;

		Vector3			_position;
		double			_size;
		std::shared_ptr<Material>	_material;
		unsigned int	_subSamples;
		double			_noiseScale; // The scale of the Perlin noise
		unsigned int	_seed;

		Perlin			_perlin;
};
