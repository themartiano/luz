#pragma once

#include "Vector3.hpp"
#include "PDFs/PDF.hpp"
#include <cmath>
#include <memory>

class   Ray
{
	public:
		Ray(void)
			: _origin(0.0, 0.0, 0.0), _direction(0.0, 0.0, 0.0), _inverseDirection(_inverse(_direction))
		{}

		Ray(const Vector3& origin, const Vector3& direction) : _origin(origin)
		{
			this->setDirection(direction);
		}

		static Ray	fromNormalizedDirection(const Vector3& origin, const Vector3& direction)
		{
			return (Ray(origin, direction, DirectionIsNormalized()));
		}

		Vector3 pointAtRay(double t) const
		{
			return (this->_origin + (this->_direction * t));
		}

		const Vector3&	getOrigin(void) const { return (this->_origin); }
		void			setOrigin(const Vector3& origin) { this->_origin = origin; }
		const Vector3&	getDirection(void) const { return (this->_direction); }
		const Vector3&	getInverseDirection(void) const { return (this->_inverseDirection); }
		void			setDirection(const Vector3& direction)
		{
			this->_direction = _normalize(direction);
			this->_inverseDirection = _inverse(this->_direction);
		}

	private:
		struct	DirectionIsNormalized {};

		Ray(const Vector3& origin, const Vector3& direction, DirectionIsNormalized)
			: _origin(origin), _direction(direction), _inverseDirection(_inverse(_direction))
		{}

		Vector3 _origin;
		Vector3 _direction;
		Vector3 _inverseDirection;

		static Vector3	_normalize(const Vector3& direction)
		{
			const double length = std::sqrt(
				(direction[0] * direction[0]) +
				(direction[1] * direction[1]) +
				(direction[2] * direction[2])
			);
			if (!std::isfinite(length) || length <= 0.0)
			{
				return (Vector3(0.0, 0.0, 0.0));
			}

			return (direction / length);
		}

		static double	_inverseComponent(double component)
		{
			if (component == 0.0 || !std::isfinite(component))
			{
				return (0.0);
			}
			const double inverse = 1.0 / component;
			return (std::isfinite(inverse) ? inverse : 0.0);
		}

		static Vector3	_inverse(const Vector3& direction)
		{
			return (Vector3(
				_inverseComponent(direction[0]),
				_inverseComponent(direction[1]),
				_inverseComponent(direction[2])
			));
		}
};
