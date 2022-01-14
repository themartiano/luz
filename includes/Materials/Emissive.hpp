#include "Materials/Material.hpp"

class	Emissive : public Material
{
	public:
		Emissive(void);
		Emissive(Color color);
		Emissive(double intensity);
		Emissive(Color color, double intensity);
		Color	emitted(Ray& ray);
		void	setIntensity(double newIntensity);

		MaterialType	type = EMISSIVE;

	private:
		double	_intensity;
};
