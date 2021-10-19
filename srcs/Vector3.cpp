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

float	Vector3::getX(void) const
{
	return (this->_x);
}

void	Vector3::setX(float x)
{
	this->_x = x;
}

float	Vector3::getY(void) const
{
	return (this->_y);
}

void	Vector3::setY(float y)
{
	this->_y = y;
}

float	Vector3::getZ(void) const
{
	return (this->_z);
}

void	Vector3::setZ(float z)
{
	this->_z = z;
}

Vector3	Vector3::operator+(const Vector3 &vec) const
{
	return (Vector3(this->_x + vec.getX(), this->_y + vec.getY(), this->_z + vec.getZ()));
}

Vector3	Vector3::operator-(const Vector3 &vec) const
{
	return (Vector3(this->_x - vec.getX(), this->_y - vec.getY(), this->_z - vec.getZ()));
}

Vector3	Vector3::operator*(const float f) const
{
	return (Vector3(this->_x * f, this->_y * f, this->_z * f));
}

Vector3	Vector3::operator/(const float f) const
{
	return (Vector3(this->_x / f, this->_y / f, this->_z / f));
}

Vector3&	Vector3::operator/=(const float f)
{
	this->_x /= f;
	this->_y /= f;
	this->_z /= f;
	return (*this);
}
