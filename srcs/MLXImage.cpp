#include "MLXImage.hpp"

MLXImage::MLXImage(void)
{
	this->_img = nullptr;
	this->_address = nullptr;
	this->_bits_per_pixel = 0;
	this->_line_length = 0;
	this->_endian = 0;
}

void	MLXImage::setImg(void* img)
{
	this->_img = img;
}

void*	MLXImage::getImg(void) const
{
	return (this->_img);
}

void	MLXImage::setAddress(char *address)
{
	this->_address = address;
}

void	MLXImage::setBitsPerPixel(int bits_per_pixel)
{
	this->_bits_per_pixel = bits_per_pixel;
}

void	MLXImage::setLineLength(int line_length)
{
	this->_line_length = line_length;
}

void	MLXImage::setEndian(int endian)
{
	this->_endian = endian;
}
