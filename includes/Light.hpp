#ifndef LIGHT_HPP
# define LIGHT_HPP

#include "Color.hpp"
#include "Transform.hpp"

class	Light
{
	public:
		Light(void);
		Light(Color color, Transform transform, float brightness);

	private:
		Color	_color;
		Transform	_transform;
		float	_brightness;

};

#endif