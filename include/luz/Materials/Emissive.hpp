#include "Materials/Material.hpp"

class	Emissive : public Material
{
	public:
		Emissive(void);
		explicit Emissive(Color radiance);
		static Emissive	fromRadiance(Color color, double radiance);
		static Emissive	fromLuminance(Color color, double luminance);
		static Emissive	fromRadiantPower(Color color, double watts, double area);
		static Emissive	fromLuminousFlux(Color color, double lumens, double area);
		Color	emitted(void);
		void	setRadiance(Color radiance);
		MaterialType	getType(void) const;
};
