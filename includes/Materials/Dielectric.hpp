#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		Dielectric(void);
		Dielectric(Color color);
		bool	scatter(Ray& ray);
		MaterialType	getType(void) const;
};
