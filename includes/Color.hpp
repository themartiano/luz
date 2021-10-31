#ifndef COLOR_HPP
#define COLOR_HPP

class	Color
{
	public:
		Color(void);
		Color(double r, double g, double b);
		double	getRed(void) const;
		void	setRed(double red);
		double	getGreen(void) const;
		void	setGreen(double green);
		double	getBlue(void) const;
		void	setBlue(double blue);
		Color&	operator+=(const Color &color2);
		Color&	operator/=(const double f);
		Color	operator+(const Color color) const;
		Color	operator*(const double f) const;
		Color	operator*(const Color color) const;
		Color	operator/(const double f) const;

	private:
		double	_red;
		double	_green;
		double	_blue;
};

#endif