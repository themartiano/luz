#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		Dielectric(void);
		Dielectric(Color color);
		virtual bool	scatter(Ray& ray);

		MaterialType	type = DIELECTRIC;
};
