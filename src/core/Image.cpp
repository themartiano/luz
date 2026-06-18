#include "Image.hpp"
#include "ColorManagement.hpp"
#include "Defaults.hpp"
#include "ImageFiles/BMP.hpp"
#include "ImageFiles/PNG.hpp"
#include "ImageFiles/TIFF.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
	double	clampUnit(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		if (value >= 1.0)
		{
			return (1.0);
		}
		return (value);
	}

	Color	scaleColor(Color color, double scale)
	{
		return (Color(
			color.getRed() * scale,
			color.getGreen() * scale,
			color.getBlue() * scale
		));
	}
}

Image::Image(void)
{
	this->_width = 0;
	this->_height = 0;
	this->_colorEncoding = ImageColorEncoding::SceneLinearACEScg;
	this->_initialized = false;
}

Image::Image(std::size_t width, std::size_t height)
{
	this->_width = width;
	this->_height = height;
	this->_colorEncoding = ImageColorEncoding::SceneLinearACEScg;
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

void	Image::saveToPNG(const std::string &fileName) const
{
	_checkInitialized();

	PNG png(fileName);
	png.writeFile(std::make_unique<Image>(*this));
}

ImageColorEncoding	Image::getColorEncoding(void) const
{
	return (this->_colorEncoding);
}

void	Image::setColorEncoding(ImageColorEncoding colorEncoding)
{
	this->_colorEncoding = colorEncoding;
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
	this->_colorEncoding = ImageColorEncoding::SceneLinearACEScg;
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

void	Image::applyExposure(double exposure)
{
	if (!std::isfinite(exposure))
	{
		throw std::invalid_argument("Exposure must be finite.");
	}

	const double exposureScale = std::pow(2.0, exposure);
	if (!std::isfinite(exposureScale))
	{
		throw std::invalid_argument("Exposure scale must be finite.");
	}

	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		this->_pixels[i] = scaleColor(ColorManagement::sanitizeSceneLinear(this->_pixels[i]), exposureScale);
	}
}

void	Image::applyContrast(double contrast)
{
	if (!std::isfinite(contrast) || contrast < 0.0)
	{
		throw std::invalid_argument("Contrast must be non-negative.");
	}

	constexpr double pivot = 0.5;
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels[i];

		pixel = Color(
			clampUnit(((pixel.getRed() - pivot) * contrast) + pivot),
			clampUnit(((pixel.getGreen() - pivot) * contrast) + pivot),
			clampUnit(((pixel.getBlue() - pivot) * contrast) + pivot)
		);
	}
}

void	Image::gammaCorrect(void)
{
	if (this->_colorEncoding == ImageColorEncoding::DisplayEncodedSRGB)
	{
		return;
	}
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels[i];

		if (this->_colorEncoding == ImageColorEncoding::SceneLinearACEScg)
		{
			pixel = ColorManagement::encodedSRGBFromACEScg(pixel);
		}
		else
		{
			pixel = ColorManagement::encodeSRGB(pixel);
		}
	}
	this->_colorEncoding = ImageColorEncoding::DisplayEncodedSRGB;
}

void	Image::toneMap(void)
{
	if (this->_colorEncoding != ImageColorEncoding::SceneLinearACEScg)
	{
		return;
	}
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels[i];

		pixel = ColorManagement::displayTransformToLinearSRGB(ColorManagement::sanitizeSceneLinear(pixel));
	}
	this->_colorEncoding = ImageColorEncoding::DisplayLinearSRGB;
}

std::unique_ptr<Image>	Image::extractBloom(double threshold, double softKnee) const
{
	if (!std::isfinite(threshold) || threshold < 0.0)
	{
		throw std::invalid_argument("Bloom threshold must be non-negative.");
	}
	if (!std::isfinite(softKnee) || softKnee < 0.0)
	{
		throw std::invalid_argument("Bloom soft knee must be non-negative.");
	}

	std::unique_ptr<Image> brightnessImage = std::make_unique<Image>(this->getWidth(), this->getHeight());
	brightnessImage->initialize();

	for (std::size_t y = 0; y < brightnessImage->getHeight(); y++)
	{
		for (std::size_t x = 0; x < brightnessImage->getWidth(); x++)
		{
			const Color pixel = ColorManagement::sanitizeSceneLinear(this->getPixel(x, y));
			const double luminance = Utilities::luminance(pixel);
			double contribution = std::max(luminance - threshold, 0.0);

			if (softKnee > 0.0)
			{
				const double knee = threshold * softKnee;
				const double soft = std::clamp(luminance - threshold + knee, 0.0, 2.0 * knee);
				contribution = std::max(contribution, (soft * soft) / ((4.0 * knee) + 0.00001));
			}

			const double weight = luminance > 0.0 ? contribution / luminance : 0.0;
			brightnessImage->setPixel(x, y, scaleColor(pixel, weight));
		}
	}

	return (brightnessImage);
}

std::unique_ptr<Image>	Image::extractBrightness(void) const
{
	return (this->extractBloom(D_BLOOM_THRESHOLD, 0.0));
}
