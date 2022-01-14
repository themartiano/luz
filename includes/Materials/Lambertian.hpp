#include "Materials/Material.hpp"

class	Lambertian : public Material
{
	public:
		Lambertian(void);
		Lambertian(Color color);
		virtual bool	scatter(Ray& ray);
		double			scatteringPDF(Ray& ray);

		MaterialType	type = LAMBERTIAN;
};
