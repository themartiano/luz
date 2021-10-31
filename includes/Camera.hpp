#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Transform.hpp"

class	Camera
{
	public:
		Camera(void);
		Camera(Vector3 lookFrom, Vector3 lookAt, short fov, double aperture);
		Vector3		getLookFrom(void) const;
		Vector3		getLookAt(void) const;
		short		getFOV(void) const;
		double		getAperture(void) const;

	private:
		Vector3		_lookFrom;
		Vector3		_lookAt;
		short		_fov; // Horizontal FOV
		double		_aperture;
};

#endif