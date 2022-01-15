#include "Materials/Material.hpp"

class	Isotropic : public Material
{
	public:
		Isotropic(void);
		Isotropic(Color color);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		double	scatteringPDF(Ray& ray, HitRecord& hitRecord);
		MaterialType	getType(void) const;
};
