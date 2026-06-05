#pragma once

#include "Vector3.hpp"
#include <iostream>

class	Color
{
	public:
		Color(void) : _red(0.0), _green(0.0), _blue(0.0) {}
		Color(double r, double g, double b) : _red(r), _green(g), _blue(b) {}

		double	getRed(void) const { return (this->_red); }
		void	setRed(double red) { this->_red = red; }
		double	getGreen(void) const { return (this->_green); }
		void	setGreen(double green) { this->_green = green; }
		double	getBlue(void) const { return (this->_blue); }
		void	setBlue(double blue) { this->_blue = blue; }

		Color&	operator+=(const Color& color2)
		{
			this->_red += color2._red;
			this->_green += color2._green;
			this->_blue += color2._blue;
			return (*this);
		}

		Color&	operator-=(const Color& color2)
		{
			this->_red -= color2._red;
			this->_green -= color2._green;
			this->_blue -= color2._blue;
			return (*this);
		}

		Color&	operator/=(const double f)
		{
			this->_red /= f;
			this->_green /= f;
			this->_blue /= f;
			return (*this);
		}

		Color	operator+(const Color& color) const
		{
			return (Color(this->_red + color._red, this->_green + color._green, this->_blue + color._blue));
		}

		Color	operator+(const double f) const
		{
			return (Color(this->_red + f, this->_green + f, this->_blue + f));
		}

		Color	operator-(const Color& color) const
		{
			return (Color(this->_red - color._red, this->_green - color._green, this->_blue - color._blue));
		}

		Color	operator*(const double f) const
		{
			return (Color(this->_red * f, this->_green * f, this->_blue * f));
		}

		Color	operator*(const Color& color) const
		{
			return (Color(this->_red * color._red, this->_green * color._green, this->_blue * color._blue));
		}

		Color	operator/(const double f) const
		{
			return (Color(this->_red / f, this->_green / f, this->_blue / f));
		}

		Color	operator/(const Color& color) const
		{
			return (Color(this->_red / color._red, this->_green / color._green, this->_blue / color._blue));
		}

		operator Vector3(void) const
		{
			return (Vector3(this->_red, this->_green, this->_blue));
		}

	private:
		double	_red;
		double	_green;
		double	_blue;
};

inline std::ostream& operator<<(std::ostream& os, const Color& color)
{
	os << color.getRed() << ", " << color.getGreen() << ", " << color.getBlue();
	return (os);
}
