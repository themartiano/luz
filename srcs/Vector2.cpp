#include "Vector2.hpp"

/*
	Constructors
*/

// Constructs the Vector2 with default values
Vector2::Vector2(void)
{
	this->_x = 0.0f;
	this->_y = 0.0f;
}

// Constructs the Vector2 with custom values
Vector2::Vector2(float x, float y)
{
	this->_x = x;
	this->_y = y;
}
