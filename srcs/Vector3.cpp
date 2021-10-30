#include "Vector3.hpp"

/*
	Constructors
*/

// Constructs the Vector3 with default values
Vector3::Vector3(void)
{
	this->_x = 0.0f;
	this->_y = 0.0f;
	this->_z = 0.0f;
}

// Constructs the Vector3 with custom values
Vector3::Vector3(float x, float y, float z)
{
	this->_x = x;
	this->_y = y;
	this->_z = z;
}

// Returns the Vector3's X axis
float	Vector3::getX(void) const
{
	return (this->_x);
}

// Sets the Vector3's X axis
void	Vector3::setX(float x)
{
	this->_x = x;
}

// Returns the Vector3's Y axis
float	Vector3::getY(void) const
{
	return (this->_y);
}

// Sets the Vector3's Y axis
void	Vector3::setY(float y)
{
	this->_y = y;
}

// Returns the Vector3's Z axis
float	Vector3::getZ(void) const
{
	return (this->_z);
}

// Sets the Vector3's Z axis
void	Vector3::setZ(float z)
{
	this->_z = z;
}

Vector3&	Vector3::operator/=(const float f)
{
	this->_x /= f;
	this->_y /= f;
	this->_z /= f;
	return (*this);
}

float	Vector3::operator[](int index) const
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
		return (0.0f);
	}
}
