#include "Materials/Material.hpp"

class	Emissive : public Material
{
	public:
		Emissive(void);
		Emissive(Color color);
		Emissive(double intensity);
		Emissive(Color color, double intensity);
		Color	emitted(void);
		void	setIntensity(double newIntensity);
		MaterialType	getType(void) const;

	private:
		double	_intensity;
};
