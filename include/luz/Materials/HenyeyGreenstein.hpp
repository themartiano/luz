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
		double	scatteringPDF(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		MaterialType	getType(void) const;

	private:
		double	_anisotropy;
};
