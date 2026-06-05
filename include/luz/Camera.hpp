#pragma once

#include "Vector3.hpp"

class	Camera
{
	public:
		Camera(void);
		Camera(Vector3 position, Vector3 direction, double fov, double aperture, double focusDistance);
		Vector3		getPosition(void) const;
		void		setPosition(Vector3 position);
		Vector3		getDirection(void) const;
		void		setDirection(Vector3 direction);
		Vector3		getUpDirection(void) const;
		void		setUpDirection(Vector3 upDirection);
		double		getFOV(void) const;
		void		setFOV(double fov);
		double		getAperture(void) const;
		void		setAperture(double aperture);
		double		getFocusDistance(void) const;
		void		setFocusDistance(double focusDistance);

	private:
		Vector3		_position;
		Vector3		_direction;
		Vector3		_upDirection;
		double		_fov; // Horizontal FOV
		double		_aperture;
		double		_focusDistance;
};
