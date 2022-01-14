#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		virtual bool	scatter(Ray& ray);
};
