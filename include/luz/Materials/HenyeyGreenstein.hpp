#pragma once

#include "Materials/Material.hpp"

class	HenyeyGreenstein : public Material
{
	public:
		HenyeyGreenstein(void);
		HenyeyGreenstein(Color color, double anisotropy);
		void	setAnisotropy(double anisotropy);
		double	getAnisotropy(void) const;
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		double	scatteringPDF(Ray& ray, HitRecord& hitRecord);
		MaterialType	getType(void) const;

	private:
		double	_anisotropy;
};
