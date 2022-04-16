#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		Dielectric(void);
		Dielectric(Color color);
		Dielectric(Color color, double refractiveIndex);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		MaterialType	getType(void) const;

	private:
		double	_refractiveIndex;
};
