#pragma once

#include "Hittables/Procedural.hpp"
#include "Noise/Perlin.hpp"
#include "Vector3.hpp"

class	Cloud : public Procedural
{
	public:
		Cloud(void);
		Cloud(Vector3 position, double size, double samplesPerSizeUnit, double noiseScale, double magnitude, double depth, unsigned int seed);
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;

	private:
		double	_getHeightAtPoint2(Vector3 point) const;
		double			sla(double x, double y, double z) const;


		unsigned int	_seed;
};
