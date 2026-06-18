#include "Materials/Material.hpp"

class	Isotropic : public Material
{
	public:
		Isotropic(void);
		Isotropic(Color color);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		double	scatteringPDF(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		MaterialType	getType(void) const;
};
