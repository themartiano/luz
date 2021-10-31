#include "Vector2.hpp"

/*
	Constructors
*/

// Constructs the Vector2 with default values
Vector2::Vector2(void)
{
	this->_x = 0.0;
	this->_y = 0.0;
}

// Constructs the Vector2 with custom values
Vector2::Vector2(double x, double y)
{
	this->_x = x;
	this->_y = y;
}
