#pragma once

#include "Materials/Material.hpp"

class	Principled : public Material
{
	public:
		Principled(void);
		Principled(Color color, double metallic, double roughness);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		double	scatteringPDF(Ray& ray, HitRecord& hitRecord);
		MaterialType	getType(void) const;

	private:
		double	_metallic;
		double	_roughness;
};
