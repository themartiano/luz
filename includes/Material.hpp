#ifndef MATERIAL_HPP
# define MATERIAL_HPP

#include "Color.hpp"

class	Material
{
	public:
		Material(void);
		Material(Color color, float opacity, float metallic, float albedo, float reflectionFuzziness, float dielectric);
		Color	getColor(void) const;
		void	setColor(Color color);
		float	getMetallic(void) const;
		float	getAlbedo(void) const;
		float	getReflectionFuzziness(void) const;
		float	getDielectric(void) const;

	private:
		Color	_color;
		float	_metallic;
		float	_albedo;
		float	_reflectionFuzziness;
		float	_dielectric;
		float	_opacity; //work on it
};

#endif