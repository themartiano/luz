#ifndef VECTOR3_HPP
# define VECTOR3_HPP

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
		Vector3		operator+(const Vector3 &vec) const;
		Vector3		operator-(const Vector3 &vec) const;
		Vector3		operator*(const float f) const;
		Vector3		operator/(const float f) const;
		Vector3&	operator/=(const float f);

	private:
		float	_x;
		float	_y;
		float	_z;
};

#endif