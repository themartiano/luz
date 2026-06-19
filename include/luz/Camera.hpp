#pragma once

#include "Vector3.hpp"

class	Camera
{
	public:
		Camera(void);
		Camera(Vector3 position, Vector3 direction, double focalLengthMeters, double sensorWidthMeters, double sensorHeightMeters, double fNumber, double focusDistanceMeters);
		Vector3		getPosition(void) const;
		void		setPosition(Vector3 position);
		Vector3		getDirection(void) const;
		void		setDirection(Vector3 direction);
		Vector3		getUpDirection(void) const;
		void		setUpDirection(Vector3 upDirection);
		double		getFocalLengthMeters(void) const;
		void		setFocalLengthMeters(double focalLengthMeters);
		double		getSensorWidthMeters(void) const;
		void		setSensorWidthMeters(double sensorWidthMeters);
		double		getSensorHeightMeters(void) const;
		void		setSensorHeightMeters(double sensorHeightMeters);
		double		getFNumber(void) const;
		void		setFNumber(double fNumber);
		double		getApertureDiameterMeters(void) const;
		void		setApertureDiameterMeters(double apertureDiameterMeters);
		bool		getPinhole(void) const;
		void		setPinhole(bool pinhole);
		double		getFocusDistanceMeters(void) const;
		void		setFocusDistanceMeters(double focusDistanceMeters);

	private:
		Vector3		_position;
		Vector3		_direction;
		Vector3		_upDirection;
		double		_focalLengthMeters;
		double		_sensorWidthMeters;
		double		_sensorHeightMeters;
		double		_fNumber;
		bool		_pinhole;
		double		_focusDistanceMeters;
};
