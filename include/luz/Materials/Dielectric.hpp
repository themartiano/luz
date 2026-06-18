#include "Materials/Material.hpp"

class	Dielectric : public Material
{
	public:
		Dielectric(void);
		Dielectric(Color color);
		Dielectric(Color color, double refractiveIndex);
		double	getRefractiveIndex(void) const;
		Color	getAbsorptionCoefficient(void) const;
		void	setAbsorptionCoefficient(Color absorptionCoefficient);
		void	setTransmittance(Color transmittance, double distanceMeters);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		MaterialType	getType(void) const;

	private:
		double	_refractiveIndex;
		Color	_absorptionCoefficient;
};
