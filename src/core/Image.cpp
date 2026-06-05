#include "Image.hpp"
#include "Defaults.hpp"
#include "ImageFiles/BMP.hpp"
#include "ImageFiles/TIFF.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <stdexcept>

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
	if (width == 0)
	{
		throw std::invalid_argument("Image width must be positive.");
	}
	this->_width = width;
}

std::size_t	Image::getHeight(void) const
{
	return (this->_height);
}

void	Image::setHeight(std::size_t height)
{
	if (height == 0)
	{
		throw std::invalid_argument("Image height must be positive.");
	}
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

Image&	Image::operator+=(const Image& other)
{
	if (this->_width != other._width || this->_height != other._height)
	{
		throw std::runtime_error("Images must have the same dimensions");
	}

	for (std::size_t i = 0; i < this->_width * this->_height; i++)
	{
		this->_pixels[i] += other._pixels[i];
	}

	return (*this);
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

void	Image::gammaCorrect(void)
{
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels[i];

		// sRGB EOTF-1
		// https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.html#TRANSFER_SRGB_INVEOTF
		pixel = Color(
			1.055 * std::pow(pixel.getRed(), 1.0 / 2.4) - 0.055,
			1.055 * std::pow(pixel.getGreen(), 1.0 / 2.4) - 0.055,
			1.055 * std::pow(pixel.getBlue(), 1.0 / 2.4) - 0.055
		);
	}
}

void	Image::toneMap(void)
{
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels[i];

		pixel = Utilities::reinhardJodie(pixel);
	}
}

std::unique_ptr<Image>	Image::extractBrightness(void) const
{
	std::unique_ptr<Image> brightnessImage = std::make_unique<Image>(this->getWidth(), this->getHeight());
	brightnessImage->initialize();

	for (std::size_t y = 0; y < brightnessImage->getHeight(); y++)
	{
		for (std::size_t x = 0; x < brightnessImage->getWidth(); x++)
		{
			double brightness = Utilities::luminance(this->getPixel(x, y));

			brightness = brightness >= 1.0 ? brightness : 0.0; // 1.0 Threshold

			brightnessImage->setPixel(x, y, Color(brightness, brightness, brightness));
		}
	}

	return (brightnessImage);
}
