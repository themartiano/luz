#include "Image.hpp"
#include "Defaults.hpp"
#include "ImageFiles/BMP.hpp"
#include "ImageFiles/TIFF.hpp"

Image::Image(void)
{
	this->_width = 0;
	this->_height = 0;
	this->_initialized = false;
}

Image::Image(std::size_t width, std::size_t height)
{
	this->_width = width;
	this->_height = height;
	this->_initialized = false;
}

std::size_t	Image::getWidth(void) const
{
	return (this->_width);
}

void	Image::setWidth(std::size_t width)
{
	this->_width = width;
}

std::size_t	Image::getHeight(void) const
{
	return (this->_height);
}

void	Image::setHeight(std::size_t height)
{
	this->_height = height;
}

Color	Image::getPixel(std::size_t x, std::size_t y) const
{
	if (x >= this->_width || y >= this->_height)
	{
		throw std::out_of_range("Index out of range");
	}

	return (this->_pixels[y * this->_width + x]);
}

void	Image::setPixel(std::size_t x, std::size_t y, Color color)
{
	if (x >= this->_width || y >= this->_height)
	{
		throw std::out_of_range("Index out of range");
	}

	this->_pixels[y * this->_width + x] = color;
}

void	Image::saveToBMP(const std::string &fileName) const
{
	_checkInitialized();

	BMP	bmp(fileName);
	bmp.writeFile(std::make_unique<Image>(*this));
}

void	Image::saveToTIFF(const std::string &fileName) const
{
	_checkInitialized();

	TIFF tiff(fileName);
	tiff.writeFile(std::make_unique<Image>(*this));
}

const SmartArray<Color>&	Image::data(void) const
{
	return (this->_pixels);
}

Color&	Image::operator[](std::size_t index)
{
	_checkInitialized();

	return (this->_pixels[index]);
}

Color&	Image::at(std::size_t index)
{
	_checkInitialized();

	if (index >= this->_pixels.getCapacity())
	{
		throw std::out_of_range("Index out of range");
	}

	return (this->_pixels[index]);
}

// Initializes the image's underlying container
void	Image::initialize(void)
{
	this->_pixels = SmartArray<Color>(this->_width * this->_height);
	this->_initialized = true;
}

// If the image is not initialized, throws an exception
void	Image::_checkInitialized(void) const
{
	if (!this->_initialized)
	{
		throw std::runtime_error("Image not initialized");
	}
}

// Assigns 'other' to this image
Image&	Image::operator=(const Image& other)
{
	this->_width = other._width;
	this->_height = other._height;
	this->_pixels = other._pixels; // SmartArray does a deep copy
	this->_initialized = other._initialized;

	return (*this);
}

// Fills itself with 'color'
void	Image::fill(Color color)
{
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		this->_pixels[i] = color;
	}
}
