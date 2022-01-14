#include "Materials/Material.hpp"

class	Metal : public Material
{
	public:
		Metal(void);
		Metal(Color color);
		Metal(double reflectionFuzziness);
		Metal(Color color, double reflectionFuzziness);
		virtual bool	scatter(Ray& ray);

		MaterialType	type = METAL;

	private:
		double	_reflectionFuzziness;
};
