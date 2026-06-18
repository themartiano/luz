#include "Materials/Material.hpp"

class	Metal : public Material
{
	public:
		Metal(void);
		Metal(Color color);
		Metal(double reflectionFuzziness);
		Metal(Color color, double reflectionFuzziness);
		Metal(Color eta, Color extinctionCoefficient, double reflectionFuzziness);
		bool	usesConductorFresnel(void) const;
		Color	getEta(void) const;
		Color	getExtinctionCoefficient(void) const;
		void	setConductorFresnel(Color eta, Color extinctionCoefficient);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		MaterialType	getType(void) const;

	private:
		double	_reflectionFuzziness;
		bool	_useConductorFresnel;
		Color	_eta;
		Color	_extinctionCoefficient;
};
