#ifndef COLOR_HPP
#define COLOR_HPP

class	Color
{
	public:
		Color(void);
		Color(float r, float g, float b);
		float	getRed(void) const;
		void	setRed(float red);
		float	getGreen(void) const;
		void	setGreen(float green);
		float	getBlue(void) const;
		void	setBlue(float blue);
		Color&	operator+=(const Color &color2);
		Color&	operator/=(const float f);
		Color	operator+(const Color color) const;
		Color	operator*(const float f) const;
		Color	operator*(const Color color) const;
		Color	operator/(const float f) const;

	private:
		float	_red;
		float	_green;
		float	_blue;
};

#endif