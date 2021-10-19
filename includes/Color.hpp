#ifndef COLOR_HPP
# define COLOR_HPP

class	Color
{
	public:
		Color(void);
		Color(int r, int g, int b, int a);
		int	getRed(void) const;
		int	getGreen(void) const;
		int	getBlue(void) const;
		Color&	operator+=(const Color &color2);
		Color&	operator/=(const int i);
		Color	operator/(const int i) const;

	private:
		int	_red;
		int	_green;
		int	_blue;
		int	_alpha;
};

#endif