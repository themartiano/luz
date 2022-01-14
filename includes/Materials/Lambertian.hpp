#include "Materials/Material.hpp"

class	Lambertian : public Material
{
	public:
		virtual bool	scatter(Ray& ray);
		double			scatteringPDF(Ray& ray);
};
