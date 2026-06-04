#include "Vector3.hpp"

/*
	Constructors
*/

// Constructs the Vector3 with default values
Vector3::Vector3(void)
{
	this->_x = 0.0;
	this->_y = 0.0;
	this->_z = 0.0;
}

// Constructs the Vector3 with custom values
Vector3::Vector3(double x, double y, double z)
{
	this->_x = x;
	this->_y = y;
	this->_z = z;
}

// Returns the Vector3's X axis
double	Vector3::getX(void) const
{
	return (this->_x);
}

// Sets the Vector3's X axis
void	Vector3::setX(double x)
{
	this->_x = x;
}

// Returns the Vector3's Y axis
double	Vector3::getY(void) const
{
	return (this->_y);
}

// Sets the Vector3's Y axis
void	Vector3::setY(double y)
{
	this->_y = y;
}

// Returns the Vector3's Z axis
double	Vector3::getZ(void) const
{
	return (this->_z);
}

// Sets the Vector3's Z axis
void	Vector3::setZ(double z)
{
	this->_z = z;
}

// (/=) Operator overload
Vector3&	Vector3::operator/=(const double f)
{
	this->_x /= f;
	this->_y /= f;
	this->_z /= f;
	return (*this);
}

// (+=) Operator overload
Vector3&	Vector3::operator+=(const Vector3 vector)
{
	this->_x += vector.getX();
	this->_y += vector.getY();
	this->_z += vector.getZ();
	return (*this);
}

// ([]) Operator overload
double	Vector3::operator[](int index) const
{
	if (index == 0)
	{
		return (this->_x);
	}
	else if (index == 1)
	{
		return (this->_y);
	}
	else if (index == 2)
	{
		return (this->_z);
	}
	else
	{
		return (0.0);
	}
}
