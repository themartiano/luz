#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		Dielectric(void);
		Dielectric(Color color);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		MaterialType	getType(void) const;
};
