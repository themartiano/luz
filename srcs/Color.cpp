#include "Color.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Color with default values
Color::Color(void)
{
	this->_red = 0.0f;
	this->_green = 0.0f;
	this->_blue = 0.0f;
}

// Constructs the Color with custom float values [ 0.0 => 1.0 ]
Color::Color(float r, float g, float b)
{
	this->_red = r;
	this->_green = g;
	this->_blue = b;
}

// Returns the Color's Red value
float	Color::getRed(void) const
{
	return (this->_red);
}

// Sets the Color's Red value
void	Color::setRed(float red)
{
	this->_red = red;
}

// Returns the Color's Green value
float	Color::getGreen(void) const
{
	return (this->_green);
}

// Sets the Color's Green value
void	Color::setGreen(float green)
{
	this->_green = green;
}

// Returns the Color's Blue value
float	Color::getBlue(void) const
{
	return (this->_blue);
}

// Sets the Color's Blue value
void	Color::setBlue(float blue)
{
	this->_blue = blue;
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
	return (Color(this->_red * f, this->_green * f, this->_blue * f));
}

Color	Color::operator*(const Color color) const
{
	return (Color(this->_red * color.getRed(), this->_green * color.getGreen(), this->_blue * color.getBlue()));
}

Color	Color::operator/(const float f) const
{
	return (Color(this->_red / f, this->_green / f, this->_blue / f));
}

Color	Color::operator+(const Color color) const
{
	return (Color(this->_red + color.getRed(), this->_green + color.getGreen(), this->_blue + color.getBlue()));
}
