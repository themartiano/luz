#ifndef CAMERA_HPP
# define CAMERA_HPP

#include "Transform.hpp"

class	Camera
{
	public:
		Camera(void);
		Camera(Transform transform, short fov);

	private:
		Transform	_transform;
		short		_fov;
		// float		half_width; //Necessario?
		// float		half_height; //Necessario?
};

#endif