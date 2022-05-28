#include "Image.hpp"
#include "Defaults.hpp"

Image::Image(void)
{
	this->_width = D_WIDTH;
	this->_height = D_HEIGHT;

	this->_pixels.reserve(this->_width * this->_height);
}

Image::Image(unsigned int width, unsigned int height)
{
	this->_width = width;
	this->_height = height;

	this->_pixels.reserve(this->_width * this->_height);
}

unsigned int	Image::getWidth(void) const
{
	return (this->_width);
}

void	Image::setWidth(unsigned int width)
{
	this->_width = width;

	this->_pixels.reserve(this->_width * this->_height);
}

unsigned int	Image::getHeight(void) const
{
	return (this->_height);
}

void	Image::setHeight(unsigned int height)
{
	this->_height = height;

	this->_pixels.reserve(this->_width * this->_height);
}

Color	Image::getPixel(unsigned int x, unsigned int y) const
{
	if (x >= this->_width || y >= this->_height)
	{
		return (Color()); // Maybe throw?
	}

	return (this->_pixels[y * this->_width + x]);
}

void	Image::setPixel(unsigned int x, unsigned int y, Color color)
{
	if (x >= this->_width || y >= this->_height)
	{
		return; // Maybe throw?
	}

	this->_pixels[y * this->_width + x] = color;
}
