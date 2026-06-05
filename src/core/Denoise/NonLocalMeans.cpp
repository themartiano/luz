#include "Denoise/NonLocalMeans.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
	std::size_t	clampCoordinate(long long value, std::size_t limit)
	{
		if (value < 0)
		{
			return (0);
		}
		if (static_cast<std::size_t>(value) >= limit)
		{
			return (limit - 1);
		}
		return (static_cast<std::size_t>(value));
	}

	double	safeChannel(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		return (std::log1p(value));
	}

	Color	featureColor(const Color& color)
	{
		return (Color(
			safeChannel(color.getRed()),
			safeChannel(color.getGreen()),
			safeChannel(color.getBlue())
		));
	}

	double	colorDistanceSquared(const Color& color1, const Color& color2)
	{
		const double red = color1.getRed() - color2.getRed();
		const double green = color1.getGreen() - color2.getGreen();
		const double blue = color1.getBlue() - color2.getBlue();

		return ((red * red) + (green * green) + (blue * blue));
	}

	double	patchDistanceSquared(
		const Image& image,
		std::size_t x1,
		std::size_t y1,
		std::size_t x2,
		std::size_t y2,
		unsigned int patchRadius
	)
	{
		double distance = 0.0;
		unsigned int samples = 0;
		const long long radius = static_cast<long long>(patchRadius);

		for (long long offsetY = -radius; offsetY <= radius; offsetY++)
		{
			for (long long offsetX = -radius; offsetX <= radius; offsetX++)
			{
				const Color color1 = featureColor(image.getPixel(
					clampCoordinate(static_cast<long long>(x1) + offsetX, image.getWidth()),
					clampCoordinate(static_cast<long long>(y1) + offsetY, image.getHeight())
				));
				const Color color2 = featureColor(image.getPixel(
					clampCoordinate(static_cast<long long>(x2) + offsetX, image.getWidth()),
					clampCoordinate(static_cast<long long>(y2) + offsetY, image.getHeight())
				));

				distance += colorDistanceSquared(color1, color2);
				samples++;
			}
		}

		return (distance / (static_cast<double>(samples) * 3.0));
	}

	Color	filterPixel(
		const Image& image,
		std::size_t x,
		std::size_t y,
		const Denoise::NonLocalMeansSettings& settings
	)
	{
		Color result(0.0, 0.0, 0.0);
		double totalWeight = 0.0;
		const long long searchRadius = static_cast<long long>(settings.searchRadius);
		const double strengthSquared = settings.strength * settings.strength;
		const double spatialSigma = std::max(1.0, static_cast<double>(settings.searchRadius) * 0.5);
		const double spatialDenominator = 2.0 * spatialSigma * spatialSigma;

		for (long long offsetY = -searchRadius; offsetY <= searchRadius; offsetY++)
		{
			for (long long offsetX = -searchRadius; offsetX <= searchRadius; offsetX++)
			{
				const std::size_t sampleX = clampCoordinate(static_cast<long long>(x) + offsetX, image.getWidth());
				const std::size_t sampleY = clampCoordinate(static_cast<long long>(y) + offsetY, image.getHeight());
				const double patchDistance = patchDistanceSquared(
					image,
					x,
					y,
					sampleX,
					sampleY,
					settings.patchRadius
				);
				const double spatialDistance = static_cast<double>((offsetX * offsetX) + (offsetY * offsetY));
				const double weight =
					std::exp(-patchDistance / strengthSquared)
					* std::exp(-spatialDistance / spatialDenominator);

				result += image.getPixel(sampleX, sampleY) * weight;
				totalWeight += weight;
			}
		}

		if (totalWeight <= std::numeric_limits<double>::epsilon())
		{
			return (image.getPixel(x, y));
		}
		return (result / totalWeight);
	}
}

void	Denoise::apply(Image& image, const NonLocalMeansSettings& settings)
{
	if (image.getWidth() == 0 || image.getHeight() == 0)
	{
		return;
	}
	if (!std::isfinite(settings.strength) || settings.strength <= 0.0)
	{
		throw std::invalid_argument("Denoise strength must be positive.");
	}

	Image denoisedImage(image.getWidth(), image.getHeight());
	denoisedImage.initialize();

	for (std::size_t y = 0; y < image.getHeight(); y++)
	{
		for (std::size_t x = 0; x < image.getWidth(); x++)
		{
			denoisedImage.setPixel(x, y, filterPixel(image, x, y, settings));
		}
	}

	image = denoisedImage;
}
