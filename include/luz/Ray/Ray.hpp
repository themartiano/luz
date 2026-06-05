#pragma once

#include "Vector3.hpp"
#include "PDFs/PDF.hpp"
#include <cmath>
#include <memory>

class   Ray
{
	public:
		Ray(void) : _origin(0.0, 0.0, 0.0), _direction(0.0, 0.0, 0.0) {}
		Ray(const Vector3& origin, const Vector3& direction) : _origin(origin), _direction(_normalize(direction)) {}

		Vector3 pointAtRay(double t) const
		{
			return (this->_origin + (this->_direction * t));
		}

		const Vector3&	getOrigin(void) const { return (this->_origin); }
		void			setOrigin(const Vector3& origin) { this->_origin = origin; }
		const Vector3&	getDirection(void) const { return (this->_direction); }
		void			setDirection(const Vector3& direction) { this->_direction = _normalize(direction); }

	private:
		Vector3 _origin;
		Vector3 _direction;

		static Vector3	_normalize(const Vector3& direction)
		{
			const double length = std::sqrt(
				(direction[0] * direction[0]) +
				(direction[1] * direction[1]) +
				(direction[2] * direction[2])
			);

			return (direction / length);
		}
};
