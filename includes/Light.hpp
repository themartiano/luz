#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "Color.hpp"
#include "Transform.hpp"

class	Light
{
	public:
		Light(void);
		Light(Color color, Transform transform, double brightness);

	private:
		Color		_color;
		Transform	_transform;
		double		_brightness;
};

#endif