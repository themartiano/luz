#pragma once

#include "Color.hpp"
#include <array>

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
		const std::array<Color>&	data(void) const;
		void			saveToBMP(const std::string &filename) const;
		void			saveToTIFF(const std::string &filename) const;

		Color&			operator[](unsigned int index);
		Color&			at(unsigned int index);

	private:
		std::array<Color>	_pixels;
		unsigned int		_width;
		unsigned int		_height;
};
