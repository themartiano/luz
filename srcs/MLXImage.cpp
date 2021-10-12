#include "MLXImage.hpp"

MLXImage::MLXImage(void)
{
	this->_img = nullptr;
	this->_address = nullptr;
	this->_bits_per_pixel = 0;
	this->_line_length = 0;
	this->_endian = 0;
}
