#ifndef MATERIAL_HPP
# define MATERIAL_HPP

#include "Color.hpp"

class	Material
{
	public:
		Material(void);
		Material(Color color, float opacity, float metallic, float albedo);
		Color	getColor(void) const;
		float	getMetallic(void) const;
		float	getAlbedo(void) const;

	private:
		Color	_color;
		float	_metallic;
		float	_albedo;
		float	_opacity; //work on it
};

#endif