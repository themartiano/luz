#include "Color.hpp"

/*
	Constructors
*/

// Constructs the Color with default values
Color::Color(void)
{
	this->_red = 0.0f;
	this->_green = 0.0f;
	this->_blue = 0.0f;
	this->_alpha = 0.0f;
}

// Constructs the Color with custom values
Color::Color(float r, float g, float b, float a)
{
	this->_red = r;
	this->_green = g;
	this->_blue = b;
	this->_alpha = a;
}

// Returns the Color's Red value
float	Color::getRed(void) const
{
	return (this->_red);
}

// Returns the Color's Green value
float	Color::getGreen(void) const
{
	return (this->_green);
}

// Returns the Color's Blue value
float	Color::getBlue(void) const
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

Color&	Color::operator/=(const float f)
{
	this->_red /= f;
	this->_green /= f;
	this->_blue /= f;
	return (*this);
}

Color	Color::operator*(const float f) const
{
	return (Color(this->_red * f, this->_green * f, this->_blue * f, 0));
}

Color	Color::operator/(const float f) const
{
	return (Color(this->_red / f, this->_green / f, this->_blue / f, 0));
}

Color	Color::operator+(const Color color) const
{
	return (Color(this->_red + color.getRed(), this->_green + color.getGreen(), this->_blue + color.getBlue(), 0.0f));
}
