#pragma once

#include "SmartArray.hpp"
#include "Color.hpp"

class	Image
{
	public:
		Image(void);
		Image(const Image &image);
		Image(std::size_t width, std::size_t height);
		std::size_t	getWidth(void) const;
		void		setWidth(std::size_t width);
		std::size_t	getHeight(void) const;
		void		setHeight(std::size_t height);
		Color		getPixel(std::size_t x, std::size_t y) const;
		void		setPixel(std::size_t x, std::size_t y, Color color);
		const SmartArray<Color>&	data(void) const;
		void		saveToBMP(const std::string &filename) const;
		void		saveToTIFF(const std::string &filename) const;
		void		initialize(void);
		Color&		at(std::size_t index);
		void		fill(Color color);

		Image&	operator=(const Image& other);
		Color&	operator[](std::size_t index);

	private:
		SmartArray<Color>	_pixels;
		std::size_t			_width;
		std::size_t			_height;
		bool				_initialized;

		void	_checkInitialized(void) const;
};
