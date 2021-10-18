#ifndef VECTOR3_HPP
# define VECTOR3_HPP

class	Vector3
{
	public:
		Vector3(void);
		Vector3(float x, float y, float z);
		float	getX(void) const;
		float	getY(void) const;
		float	getZ(void) const;
		Vector3&	operator+(void);
		Vector3		operator*(const float f);

	private:
		float	_x;
		float	_y;
		float	_z;
};

#endif