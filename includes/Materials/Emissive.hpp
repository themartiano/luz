#include "Materials/Material.hpp"

class	Emissive : public Material
{
	public:
		Color	emitted(Ray& ray);
		void	setIntensity(double newIntensity);

	private:
		double	_intensity = 1.0;
};
