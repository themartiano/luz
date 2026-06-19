#include "Materials/Material.hpp"

class	Lambertian : public Material
{
	public:
		Lambertian(void);
		Lambertian(Color color);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		double	scatteringPDF(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		MaterialType	getType(void) const;
};
