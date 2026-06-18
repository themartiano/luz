#pragma once

#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		Dielectric(void);
		Dielectric(Color color);
		Dielectric(Color color, double refractiveIndex);
		Dielectric(Color color, double refractiveIndex, double roughness);
		double	getRefractiveIndex(void) const;
		double	getRoughness(void) const;
		Color	getAbsorptionCoefficient(void) const;
		void	setRoughness(double roughness);
		void	setAbsorptionCoefficient(Color absorptionCoefficient);
		void	setTransmittance(Color transmittance, double distanceMeters);
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
		double	_refractiveIndex;
		double	_roughness;
		Color	_absorptionCoefficient;
};
