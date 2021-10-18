#include "Color.hpp"

/*
	Constructors
*/

// Constructs the Color with default values
Color::Color(void)
{
	this->_red = 0;
	this->_green = 0;
	this->_blue = 0;
	this->_alpha = 0;
}

// Constructs the Color with custom values
Color::Color(short r, short g, short b, short a)
{
	this->_red = r;
	this->_green = g;
	this->_blue = b;
	this->_alpha = a;
}

short	Color::getRed(void) const
{
	return (this->_red);
}

short	Color::getGreen(void) const
{
	return (this->_green);
}

short	Color::getBlue(void) const
{
	return (this->_blue);
}
