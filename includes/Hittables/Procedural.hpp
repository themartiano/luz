#pragma once

#include "Hittables/Hittable.hpp"
#include "Noise/Perlin.hpp"

class	Procedural : public Hittable
{
	public:
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;

	protected:
		double	_getHeightAtPoint(double x, double z) const;
		Vector3	_getNormalAtPosition(Vector3 position, double t_min) const;

		Vector3			_position;
		double			_size;
		std::shared_ptr<Material>	_material;
		unsigned int	_samplesPerSizeUnit;
		double			_noiseScale; // The scale of the Perlin noise
		double			_magnitude; // Noise height multiplier
		double			_depth; // How much to render below the Y position (local 0.0)

		Perlin			_perlin;
};
