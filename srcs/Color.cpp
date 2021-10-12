#include "Color.hpp"

/*
	Constructors
*/

Color::Color(void)
{
	this->_red = 0;
	this->_green = 0;
	this->_blue = 0;
	this->_alpha = 0;
}

Color::Color(short r, short g, short b, short a)
{
	this->_red = r;
	this->_green = g;
	this->_blue = b;
	this->_alpha = a;
}
