#include "Materials/Material.hpp"

class	Metal : public Material
{
	public:
		Metal(void);
		Metal(Color color);
		Metal(double reflectionFuzziness);
		Metal(Color color, double reflectionFuzziness);
		bool	scatter(Ray& ray);
		MaterialType	getType(void) const;

	private:
		double	_reflectionFuzziness;
};
