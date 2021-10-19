#ifndef MATERIAL_HPP
# define MATERIAL_HPP

#include "Color.hpp"

class	Material
{
	public:
		Material(void);
		Material(Color color, float opacity);
		Color	getColor(void) const;

	private:
		Color	_color;
		float	_opacity;
};

#endif