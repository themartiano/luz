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
Color::Color(int r, int g, int b, int a)
{
	this->_red = r;
	this->_green = g;
	this->_blue = b;
	this->_alpha = a;
}

int	Color::getRed(void) const
{
	return (this->_red);
}

int	Color::getGreen(void) const
{
	return (this->_green);
}

int	Color::getBlue(void) const
{
	return (this->_blue);
}

Color&	Color::operator+=(const Color &color2)
{
	this->_red += color2.getRed();
	this->_green += color2.getGreen();
	this->_blue += color2.getBlue();
	return (*this);
}

Color&	Color::operator/=(const int i)
{
	this->_red /= i;
	this->_green /= i;
	this->_blue /= i;
	return (*this);
}

Color	Color::operator/(const int i) const
{
	return (Color(this->_red / i, this->_green / i, this->_blue / i, 0));
}
