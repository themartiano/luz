#ifndef VECTOR3_HPP
#define VECTOR3_HPP

class	Vector3
{
	public:
		Vector3(void);
		Vector3(double x, double y, double z);
		double		getX(void) const;
		void		setX(double x);
		double		getY(void) const;
		void		setY(double y);
		double		getZ(void) const;
		void		setZ(double z);
		Vector3&	operator/=(const double f);
		Vector3&	operator+=(const Vector3 vector);
		double		operator[](int index) const;

	private:
		double	_x;
		double	_y;
		double	_z;
};

inline Vector3	operator+(const Vector3 &vector1, const Vector3 &vector2)
{
	return (Vector3(vector1[0] + vector2[0], vector1[1] + vector2[1], vector1[2] + vector2[2]));
}

inline Vector3	operator-(const Vector3 &vector1, const Vector3 &vector2)
{
	return (Vector3(vector1[0] - vector2[0], vector1[1] - vector2[1], vector1[2] - vector2[2]));
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

#endif