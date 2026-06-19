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
		double	getRoughness(void) const;
		Color	getEta(void) const;
		Color	getExtinctionCoefficient(void) const;
		void	setRoughness(double roughness);
		void	setConductorFresnel(Color eta, Color extinctionCoefficient);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		Color	evaluateBSDFCos(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		double	scatteringPDF(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		MaterialType	getType(void) const;

	private:
		double	_reflectionFuzziness;
		bool	_useConductorFresnel;
		Color	_eta;
		Color	_extinctionCoefficient;
};
