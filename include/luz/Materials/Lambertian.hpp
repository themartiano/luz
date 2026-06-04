#include "Materials/Material.hpp"

class	Lambertian : public Material
{
	public:
		Lambertian(void);
		Lambertian(Color color);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		double	scatteringPDF(Ray& ray, HitRecord& hitRecord);
		MaterialType	getType(void) const;
};
