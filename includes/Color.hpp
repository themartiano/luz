#ifndef COLOR_HPP
# define COLOR_HPP

class	Color
{
	public:
		Color(void);
		Color(short r, short g, short b, short a);
		short	getRed(void) const;
		short	getGreen(void) const;
		short	getBlue(void) const;

	private:
		short	_red;
		short	_green;
		short	_blue;
		short	_alpha;
};

#endif