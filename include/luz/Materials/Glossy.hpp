#pragma once

#include "Materials/Material.hpp"

class	Glossy : public Material
{
	public:
		Glossy(void);
		Glossy(Color color, double roughness);
		double	getRoughness(void) const;
		void	setRoughness(double roughness);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		Color	evaluateBSDFCos(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		double	scatteringPDF(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		MaterialType	getType(void) const;

	private:
		double	_roughness;
};
