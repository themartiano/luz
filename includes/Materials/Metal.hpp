#include "Materials/Material.hpp"

class	Metal : public Material
{
	public:
		virtual bool	scatter(Ray& ray);

	private:
		double	_reflectionFuzziness;
};
