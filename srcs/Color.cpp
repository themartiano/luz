#include "Color.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Color with default values
Color::Color(void)
{
	this->_red = 0.0;
	this->_green = 0.0;
	this->_blue = 0.0;
}

// Constructs the Color with custom double values [ 0.0 => 1.0 ]
Color::Color(double r, double g, double b)
{
	this->_red = r;
	this->_green = g;
	this->_blue = b;
}

// Returns the Color's Red value
double	Color::getRed(void) const
{
	return (this->_red);
}

// Sets the Color's Red value
void	Color::setRed(double red)
{
	this->_red = red;
}

// Returns the Color's Green value
double	Color::getGreen(void) const
{
	return (this->_green);
}

// Sets the Color's Green value
void	Color::setGreen(double green)
{
	this->_green = green;
}

// Returns the Color's Blue value
double	Color::getBlue(void) const
{
	return (this->_blue);
}

// Sets the Color's Blue value
void	Color::setBlue(double blue)
{
	this->_blue = blue;
}

// (+=) Operator overload
Color&	Color::operator+=(const Color &color2)
{
	this->_red += color2.getRed();
	this->_green += color2.getGreen();
	this->_blue += color2.getBlue();
	return (*this);
}

// (-=) Operator overload
Color&	Color::operator-=(const Color &color2)
{
	this->_red -= color2.getRed();
	this->_green -= color2.getGreen();
	this->_blue -= color2.getBlue();
	return (*this);
}

// (/=) Operator overload
Color&	Color::operator/=(const double f)
{
	this->_red /= f;
	this->_green /= f;
	this->_blue /= f;
	return (*this);
}

// (*) Operator overload
Color	Color::operator*(const double f) const
{
	return (Color(this->_red * f, this->_green * f, this->_blue * f));
}

// (*) Operator overload
Color	Color::operator*(const Color color) const
{
	return (Color(this->_red * color.getRed(), this->_green * color.getGreen(), this->_blue * color.getBlue()));
}

// (/) Operator overload
Color	Color::operator/(const double f) const
{
	return (Color(this->_red / f, this->_green / f, this->_blue / f));
}

// (/) Operator overload
Color	Color::operator/(const Color color) const
{
	return (Color(this->_red / color.getRed(), this->_green / color.getGreen(), this->_blue / color.getBlue()));
}

// (+) Operator overload
Color	Color::operator+(const Color color) const
{
	return (Color(this->_red + color.getRed(), this->_green + color.getGreen(), this->_blue + color.getBlue()));
}

// (+) Operator overload
Color	Color::operator+(const double f) const
{
	return (Color(this->_red + f, this->_green + f, this->_blue + f));
}

// (-) Operator overload
Color	Color::operator-(const Color color) const
{
	return (Color(this->_red - color.getRed(), this->_green - color.getGreen(), this->_blue - color.getBlue()));
}

Color::operator Vector3(void) const
{
	return (Vector3(this->_red, this->_green, this->_blue));
}
