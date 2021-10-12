#ifndef TRANSFORM_HPP
# define TRANSFORM_HPP

#include "Vector3.hpp"

class	Transform
{
	public:
		Transform(void);
		Transform(Vector3 position, Vector3 orientation, Vector3 scale);

	private:
		Vector3	_position;
		// Vector3	_rotation;
		Vector3	_orientation;
		Vector3	_scale;

};

#endif