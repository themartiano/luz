#include "Vector3.hpp"

/*
	Constructors
*/

Vector3::Vector3(void)
{
	this->_x = 0.0f;
	this->_y = 0.0f;
	this->_z = 0.0f;
}

Vector3::Vector3(float x, float y, float z)
{
	this->_x = x;
	this->_y = y;
	this->_z = z;
}
