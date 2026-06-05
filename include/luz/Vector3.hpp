#pragma once

#include <iostream>

class	Vector3
{
	public:
		Vector3(void) : _x(0.0), _y(0.0), _z(0.0) {}
		Vector3(double x, double y, double z) : _x(x), _y(y), _z(z) {}

		double		getX(void) const { return (this->_x); }
		void		setX(double x) { this->_x = x; }
		double		getY(void) const { return (this->_y); }
		void		setY(double y) { this->_y = y; }
		double		getZ(void) const { return (this->_z); }
		void		setZ(double z) { this->_z = z; }

		Vector3&	operator/=(const double f)
		{
			this->_x /= f;
			this->_y /= f;
			this->_z /= f;
			return (*this);
		}

		Vector3&	operator+=(const Vector3& vector)
		{
			this->_x += vector._x;
			this->_y += vector._y;
			this->_z += vector._z;
			return (*this);
		}

		double		operator[](int index) const
		{
			if (index == 0)
			{
				return (this->_x);
			}
			if (index == 1)
			{
				return (this->_y);
			}
			if (index == 2)
			{
				return (this->_z);
			}
			return (0.0);
		}

	private:
		double	_x;
		double	_y;
		double	_z;
};

inline Vector3	operator+(const Vector3 &vector1, const Vector3 &vector2)
{
	return (Vector3(vector1[0] + vector2[0], vector1[1] + vector2[1], vector1[2] + vector2[2]));
}

inline Vector3	operator+(const Vector3 &vector, const double f)
{
	return (Vector3(vector[0] + f, vector[1] + f, vector[2] + f));
}

inline Vector3	operator-(const Vector3 &vector1, const Vector3 &vector2)
{
	return (Vector3(vector1[0] - vector2[0], vector1[1] - vector2[1], vector1[2] - vector2[2]));
}

inline Vector3	operator-(const Vector3 &vector, const double f)
{
	return (Vector3(vector[0] - f, vector[1] - f, vector[2] - f));
}

inline Vector3 operator*(const double f, const Vector3& vector)
{
	return (Vector3(f * vector[0], f * vector[1], f * vector[2]));
}

inline Vector3 operator*(const Vector3& vector, const double f)
{
	return (f * vector);
}

inline Vector3 operator*(const Vector3& vector1, const Vector3& vector2)
{
	return (Vector3(vector1.getX() * vector2.getX(), vector1.getY() * vector2.getY(), vector1.getZ() * vector2.getZ()));
}

inline Vector3 operator/(const Vector3& vector, const double f)
{
	return ((1.0 / f) * vector);
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& vector)
{
	os << vector.getX() << ", " << vector.getY() << ", " << vector.getZ();
	return (os);
}

inline bool operator==(const Vector3& vector1, const Vector3& vector2)
{
	return (vector1.getX() == vector2.getX() && vector1.getY() == vector2.getY() && vector1.getZ() == vector2.getZ());
}

inline bool operator!=(const Vector3& vector1, const Vector3& vector2)
{
	return (vector1.getX() != vector2.getX() || vector1.getY() != vector2.getY() || vector1.getZ() != vector2.getZ());
}
