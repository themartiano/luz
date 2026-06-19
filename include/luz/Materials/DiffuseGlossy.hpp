#pragma once

#include "Materials/Material.hpp"

class	DiffuseGlossy : public Material
{
	public:
		DiffuseGlossy(void);
		DiffuseGlossy(Color diffuseColor, Color glossyColor, double glossyWeight, double glossyRoughness);
		Color	getGlossyColor(void) const;
		double	getGlossyWeight(void) const;
		double	getGlossyRoughness(void) const;
		void	setGlossyColor(Color glossyColor);
		void	setGlossyWeight(double glossyWeight);
		void	setGlossyRoughness(double glossyRoughness);
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
		Color	_glossyColor;
		double	_glossyWeight;
		double	_glossyRoughness;
};
