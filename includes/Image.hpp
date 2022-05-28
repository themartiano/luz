#pragma once

#include "Color.hpp"
#include <vector>

class	Image
{
	public:
		Image(void);
		Image(unsigned int width, unsigned int height);
		unsigned int	getWidth(void) const;
		void			setWidth(unsigned int width);
		unsigned int	getHeight(void) const;
		void			setHeight(unsigned int height);
		Color			getPixel(unsigned int x, unsigned int y) const;
		void			setPixel(unsigned int x, unsigned int y, Color color);

	private:
		std::vector<Color>	_pixels;
		unsigned int		_width;
		unsigned int		_height;
};
