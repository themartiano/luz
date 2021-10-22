#ifndef CAMERA_HPP
# define CAMERA_HPP

#include "Transform.hpp"

class	Camera
{
	public:
		Camera(void);
		Camera(Transform transform, short fov, float aperture);
		Transform	getTransform(void) const;
		short		getFOV(void) const;
		float		getAperture(void) const;

	private:
		Transform	_transform;
		short		_fov; // Horizontal FOV
		float		_aperture;
};

#endif