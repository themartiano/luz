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
#include <vector>

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

	double	sceneLuminance(Color color)
	{
		return (Utilities::luminance(ColorManagement::sanitizeSceneLinear(color)));
	}

	Color	suppressIsolatedBloomFirefly(const Image& image, std::size_t x, std::size_t y, Color pixel, double threshold)
	{
		constexpr double HOT_MIN_THRESHOLD_MULTIPLIER = 16.0;
		constexpr double HOT_NEIGHBOR_RATIO = 16.0;
		constexpr double HOT_NEIGHBOR_CEILING_MULTIPLIER = 4.0;
		constexpr int RADIUS = 2;
		const double luminance = sceneLuminance(pixel);

		if (threshold <= 0.0 || luminance <= threshold * HOT_MIN_THRESHOLD_MULTIPLIER)
		{
			return (pixel);
		}

		double neighborMax = 0.0;
		unsigned int similarNeighbors = 0;
		unsigned int brightNeighbors = 0;
		for (int offsetY = -RADIUS; offsetY <= RADIUS; offsetY++)
		{
			for (int offsetX = -RADIUS; offsetX <= RADIUS; offsetX++)
			{
				if (offsetX == 0 && offsetY == 0)
				{
					continue;
				}
				const long long sampleX = static_cast<long long>(x) + offsetX;
				const long long sampleY = static_cast<long long>(y) + offsetY;
				if (
					sampleX < 0
					|| sampleY < 0
					|| static_cast<std::size_t>(sampleX) >= image.getWidth()
					|| static_cast<std::size_t>(sampleY) >= image.getHeight()
				)
				{
					continue;
				}
				const double neighborLuminance = sceneLuminance(image.getPixelUnchecked(
					static_cast<std::size_t>(sampleX),
					static_cast<std::size_t>(sampleY)
				));

				neighborMax = std::max(neighborMax, neighborLuminance);
				if (neighborLuminance >= luminance / HOT_NEIGHBOR_RATIO)
				{
					similarNeighbors++;
				}
				if (neighborLuminance > threshold)
				{
					brightNeighbors++;
				}
			}
		}

		if (similarNeighbors > 0 || brightNeighbors >= 3)
		{
			return (pixel);
		}

		const double luminanceCeiling = std::max(
			threshold,
			neighborMax * HOT_NEIGHBOR_CEILING_MULTIPLIER
		);
		if (luminance <= luminanceCeiling)
		{
			return (pixel);
		}
		return (scaleColor(pixel, luminanceCeiling / luminance));
	}

	double	maxChannel(Color color)
	{
		return (std::max(color.getRed(), std::max(color.getGreen(), color.getBlue())));
	}

	double	minChannel(Color color)
	{
		return (std::min(color.getRed(), std::min(color.getGreen(), color.getBlue())));
	}

	bool	isDisplayFireflyCandidate(Color color)
	{
		return (
			maxChannel(color) >= 0.99
			&& minChannel(color) >= 0.85
			&& sceneLuminance(color) >= 0.95
		);
	}

	bool	isBrightDisplayOutlierCandidate(Color color)
	{
		return (
			maxChannel(color) >= 0.50
			&& sceneLuminance(color) >= 0.35
		);
	}

	bool	isUnsupportedSaturatedDisplayPixel(Color color)
	{
		return (
			maxChannel(color) >= 0.995
			&& minChannel(color) >= 0.96
			&& sceneLuminance(color) >= 0.98
		);
	}

	Color	medianNeighborColor(std::vector<Color>& colors)
	{
		std::sort(
			colors.begin(),
			colors.end(),
			[](Color a, Color b)
			{
				return (sceneLuminance(a) < sceneLuminance(b));
			}
		);
		return (colors[colors.size() / 2]);
	}

	Color	suppressDisplayFirefly(const Image& image, std::size_t x, std::size_t y)
	{
		constexpr int RADIUS = 2;
		constexpr double SIMILAR_NEIGHBOR_RATIO = 0.65;
		constexpr double BRIGHT_NEIGHBOR_THRESHOLD = 0.80;
		constexpr double LOCAL_OUTLIER_RATIO = 1.65;
		constexpr double LOCAL_OUTLIER_EXCESS = 0.08;
		constexpr unsigned int MIN_SIMILAR_SUPPORT = 4;
		constexpr unsigned int MIN_BRIGHT_SUPPORT = 6;
		constexpr unsigned int MIN_SATURATED_SUPPORT = 3;
		const Color pixel = image.getPixelUnchecked(x, y);
		const double luminance = sceneLuminance(pixel);
		const bool saturatedCandidate = isUnsupportedSaturatedDisplayPixel(pixel);
		const bool displayCandidate = isDisplayFireflyCandidate(pixel);

		if (!displayCandidate && !isBrightDisplayOutlierCandidate(pixel))
		{
			return (pixel);
		}

		unsigned int similarNeighbors = 0;
		unsigned int brightNeighbors = 0;
		unsigned int saturatedNeighbors = 0;
		std::vector<double> immediateNeighborLuminances;
		std::vector<Color> replacementCandidates;
		immediateNeighborLuminances.reserve(8);
		for (int offsetY = -RADIUS; offsetY <= RADIUS; offsetY++)
		{
			for (int offsetX = -RADIUS; offsetX <= RADIUS; offsetX++)
			{
				if (offsetX == 0 && offsetY == 0)
				{
					continue;
				}
				const long long sampleX = static_cast<long long>(x) + offsetX;
				const long long sampleY = static_cast<long long>(y) + offsetY;
				if (
					sampleX < 0
					|| sampleY < 0
					|| static_cast<std::size_t>(sampleX) >= image.getWidth()
					|| static_cast<std::size_t>(sampleY) >= image.getHeight()
				)
				{
					continue;
				}

				const Color neighbor = image.getPixelUnchecked(
					static_cast<std::size_t>(sampleX),
					static_cast<std::size_t>(sampleY)
				);
				const double neighborLuminance = sceneLuminance(neighbor);
				if (std::abs(offsetX) <= 1 && std::abs(offsetY) <= 1)
				{
					immediateNeighborLuminances.push_back(neighborLuminance);
				}
				if (neighborLuminance >= luminance * SIMILAR_NEIGHBOR_RATIO)
				{
					similarNeighbors++;
				}
				if (neighborLuminance >= BRIGHT_NEIGHBOR_THRESHOLD)
				{
					brightNeighbors++;
				}
				if (isUnsupportedSaturatedDisplayPixel(neighbor))
				{
					saturatedNeighbors++;
				}
				if (!isDisplayFireflyCandidate(neighbor))
				{
					replacementCandidates.push_back(neighbor);
				}
			}
		}

		if (!displayCandidate)
		{
			if (immediateNeighborLuminances.empty())
			{
				return (pixel);
			}
			std::sort(immediateNeighborLuminances.begin(), immediateNeighborLuminances.end());
			const double localMedian = immediateNeighborLuminances[immediateNeighborLuminances.size() / 2];
			if (
				luminance < localMedian * LOCAL_OUTLIER_RATIO
				|| luminance < localMedian + LOCAL_OUTLIER_EXCESS
			)
			{
				return (pixel);
			}
			if (!replacementCandidates.empty())
			{
				return (medianNeighborColor(replacementCandidates));
			}
		}
		if (
			saturatedCandidate
			&& saturatedNeighbors < MIN_SATURATED_SUPPORT
			&& !replacementCandidates.empty()
		)
		{
			return (medianNeighborColor(replacementCandidates));
		}
		if (similarNeighbors >= MIN_SIMILAR_SUPPORT || brightNeighbors >= MIN_BRIGHT_SUPPORT)
		{
			return (pixel);
		}
		if (replacementCandidates.empty())
		{
			return (pixel);
		}
		return (medianNeighborColor(replacementCandidates));
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

	return (this->getPixelUnchecked(x, y));
}

void	Image::setPixel(std::size_t x, std::size_t y, Color color)
{
	if (x >= this->_width || y >= this->_height)
	{
		throw std::out_of_range("Index out of range");
	}

	this->setPixelUnchecked(x, y, color);
}

Color	Image::getPixelUnchecked(std::size_t x, std::size_t y) const
{
	return (this->_pixels.unchecked(y * this->_width + x));
}

void	Image::setPixelUnchecked(std::size_t x, std::size_t y, Color color)
{
	this->_pixels.unchecked(y * this->_width + x) = color;
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

Color*	Image::pixels(void)
{
	return (this->_pixels.data());
}

const Color*	Image::pixels(void) const
{
	return (this->_pixels.data());
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

	return (this->_pixels.unchecked(index));
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
		this->_pixels.unchecked(i) += other._pixels.unchecked(i);
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
		this->_pixels.unchecked(i) = color;
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
		this->_pixels.unchecked(i) = scaleColor(ColorManagement::sanitizeSceneLinear(this->_pixels.unchecked(i)), exposureScale);
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
		Color& pixel = this->_pixels.unchecked(i);

		pixel = Color(
			clampUnit(((pixel.getRed() - pivot) * contrast) + pivot),
			clampUnit(((pixel.getGreen() - pivot) * contrast) + pivot),
			clampUnit(((pixel.getBlue() - pivot) * contrast) + pivot)
		);
	}
}

void	Image::suppressIsolatedFireflies(void)
{
	_checkInitialized();
	if (this->_colorEncoding != ImageColorEncoding::DisplayEncodedSRGB)
	{
		return;
	}
	if (this->_width < 3 || this->_height < 3)
	{
		return;
	}

	Image source = *this;
	for (std::size_t y = 0; y < this->_height; y++)
	{
		for (std::size_t x = 0; x < this->_width; x++)
		{
			this->setPixelUnchecked(x, y, suppressDisplayFirefly(source, x, y));
		}
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
		Color& pixel = this->_pixels.unchecked(i);

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

void	Image::applyViewTransform(ViewTransform viewTransform)
{
	if (
		viewTransform == ViewTransform::Raw
		|| this->_colorEncoding != ImageColorEncoding::SceneLinearACEScg
	)
	{
		return;
	}
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels.unchecked(i);

		pixel = ColorManagement::viewTransformToLinearSRGB(
			ColorManagement::sanitizeSceneLinear(pixel),
			viewTransform
		);
	}
	this->_colorEncoding = ImageColorEncoding::DisplayLinearSRGB;
}

void	Image::applyViewTransformAndEncodeSRGB(ViewTransform viewTransform)
{
	if (
		viewTransform == ViewTransform::Raw
		|| this->_colorEncoding == ImageColorEncoding::DisplayEncodedSRGB
	)
	{
		return;
	}
	if (this->_colorEncoding != ImageColorEncoding::SceneLinearACEScg)
	{
		this->gammaCorrect();
		return;
	}
	for (std::size_t i = 0; i < this->_pixels.getCapacity(); i++)
	{
		Color& pixel = this->_pixels.unchecked(i);

		pixel = ColorManagement::encodeSRGB(
			ColorManagement::viewTransformToLinearSRGB(
				ColorManagement::sanitizeSceneLinear(pixel),
				viewTransform
			)
		);
	}
	this->_colorEncoding = ImageColorEncoding::DisplayEncodedSRGB;
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
			const Color pixel = suppressIsolatedBloomFirefly(
				*this,
				x,
				y,
				ColorManagement::sanitizeSceneLinear(this->getPixelUnchecked(x, y)),
				threshold
			);
			const double luminance = Utilities::luminance(pixel);
			double contribution = std::max(luminance - threshold, 0.0);

			if (softKnee > 0.0)
			{
				const double knee = threshold * softKnee;
				const double soft = std::clamp(luminance - threshold + knee, 0.0, 2.0 * knee);
				contribution = std::max(contribution, (soft * soft) / ((4.0 * knee) + 0.00001));
			}

			const double weight = luminance > 0.0 ? contribution / luminance : 0.0;
			brightnessImage->setPixelUnchecked(x, y, scaleColor(pixel, weight));
		}
	}

	return (brightnessImage);
}

std::unique_ptr<Image>	Image::extractBrightness(void) const
{
	return (this->extractBloom(D_BLOOM_THRESHOLD, 0.0));
}
