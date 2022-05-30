#include "Image.hpp"
#include "Defaults.hpp"
#include "ImageFiles/BMP.hpp"
#include "ImageFiles/TIFF.hpp"

Image::Image(void)
{
	this->_width = D_WIDTH; // Should it be 0?
	this->_height = D_HEIGHT;  // Should it be 0?

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
		throw std::out_of_range("Index out of range");
	}

	return (this->_pixels[y * this->_width + x]);
}

void	Image::setPixel(unsigned int x, unsigned int y, Color color)
{
	if (x >= this->_width || y >= this->_height)
	{
		throw std::out_of_range("Index out of range");
	}

	this->_pixels[y * this->_width + x] = color;
}

void	Image::saveToBMP(const std::string &fileName) const
{
	BMP	bmp(fileName);
	bmp.writeFile(*this);
}

void	Image::saveToTIFF(const std::string &fileName) const
{
	TIFF tiff(fileName);
	tiff.writeFile(*this);
}

const std::vector<Color>&	Image::data(void) const
{
	return (this->_pixels);
}

Color&	Image::operator[](unsigned int index)
{
	return (this->_pixels[index]);
}

Color&	Image::at(unsigned int index)
{
	if (index >= this->_pixels.size())
	{
		throw std::out_of_range("Index out of range");
	}

	return (this->_pixels[index]);
}
