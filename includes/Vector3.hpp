#ifndef VECTOR3_HPP
#define VECTOR3_HPP

class	Vector3
{
	public:
		Vector3(void);
		Vector3(float x, float y, float z);
		float		getX(void) const;
		void		setX(float x);
		float		getY(void) const;
		void		setY(float y);
		float		getZ(void) const;
		void		setZ(float z);
		Vector3&	operator/=(const float f);
		float		operator[](int index) const;

	private:
		float	_x;
		float	_y;
		float	_z;
};

inline Vector3	operator+(const Vector3 &vector1, const Vector3 &vector2)
{
	return (Vector3(vector1[0] + vector2[0], vector1[1] + vector2[1], vector1[2] + vector2[2]));
}

inline Vector3	operator-(const Vector3 &vector1, const Vector3 &vector2)
{
	return (Vector3(vector1[0] - vector2[0], vector1[1] - vector2[1], vector1[2] - vector2[2]));
}

inline Vector3 operator*(const float f, const Vector3& vector)
{
	return (Vector3(f * vector[0], f * vector[1], f * vector[2]));
}

inline Vector3 operator*(const Vector3& vector, const float f)
{
	return (f * vector);
}

inline Vector3 operator*(const Vector3& vector1, const Vector3& vector2)
{
	return (Vector3(vector1.getX() * vector2.getX(), vector1.getY() * vector2.getY(), vector1.getZ() * vector2.getZ()));
}

inline Vector3 operator/(const Vector3& vector, const float f)
{
	return ((1.0f / f) * vector);
}

#endif