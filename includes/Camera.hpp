#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Transform.hpp"

class	Camera
{
	public:
		Camera(void);
		Camera(Vector3 position, Vector3 direction, short fov, double aperture, double focusDistance);
		Vector3		getPosition(void) const;
		Vector3		getDirection(void) const;
		short		getFOV(void) const;
		double		getAperture(void) const;
		double		getFocusDistance(void) const;

	private:
		Vector3		_position;
		Vector3		_direction;
		short		_fov; // Horizontal FOV
		double		_aperture;
		double		_focusDistance;
};

#endif