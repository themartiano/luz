#include "Blur/Gaussian.hpp"
#include <cstddef>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace
{
	std::size_t	clampIndex(long long value, std::size_t limit)
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

	std::vector<double>	createSeparableKernel(unsigned int diameter, double amount)
	{
		const int radius = static_cast<int>(diameter / 2);
		const double s = 2.0 * amount * amount;
		std::vector<double> kernel(diameter);
		double sum = 0.0;

		for (int x = -radius; x <= radius; x++)
		{
			const double value = std::exp(-(static_cast<double>(x * x)) / s);

			kernel[static_cast<std::size_t>(x + radius)] = value;
			sum += value;
		}
		for (double& value : kernel)
		{
			value /= sum;
		}
		return (kernel);
	}
}

Gaussian::Kernel	Gaussian::createKernel(unsigned int diameter, double amount)
{
	if (diameter == 0 || diameter % 2 == 0)
	{
		throw std::invalid_argument("Gaussian blur diameter must be a positive odd number.");
	}
	if (!std::isfinite(amount) || amount <= 0.0)
	{
		throw std::invalid_argument("Gaussian blur amount must be positive.");
	}

	const int radius = static_cast<int>(diameter / 2);

	Kernel kernel;

	double r;
	double s = 2.0 * amount * amount;
	double sum = 0.0;

	kernel.resize(diameter);
	for (int x = -radius; x <= radius; x++)
	{
		kernel[static_cast<std::size_t>(x + radius)].resize(diameter);

		for (int y = -radius; y <= radius; y++)
		{
			r = sqrt(x * x + y * y);
			kernel[static_cast<std::size_t>(x + radius)][static_cast<std::size_t>(y + radius)] = (exp(-(r * r) / s)) / (std::numbers::pi * s);
			sum += kernel[static_cast<std::size_t>(x + radius)][static_cast<std::size_t>(y + radius)];
		}
	}

	// normalising the Kernel
	for (unsigned int x = 0; x < diameter; x++)
	{
		for (unsigned int y = 0; y < diameter; y++)
		{
			kernel[x][y] /= sum;
		}
	}

	return (kernel);
}

void	Gaussian::blur(const Image& image, Image& blurredImage, unsigned int diameter, double amount)
{
	if (image.getWidth() != blurredImage.getWidth() || image.getHeight() != blurredImage.getHeight())
	{
		throw std::invalid_argument("Gaussian blur images must have matching dimensions.");
	}
	if (&image == &blurredImage)
	{
		Image source = image;
		blur(source, blurredImage, diameter, amount);
		return;
	}

	if (diameter == 0 || diameter % 2 == 0)
	{
		throw std::invalid_argument("Gaussian blur diameter must be a positive odd number.");
	}
	if (!std::isfinite(amount) || amount <= 0.0)
	{
		throw std::invalid_argument("Gaussian blur amount must be positive.");
	}

	const std::vector<double> kernel = createSeparableKernel(diameter, amount);

	const int radius = static_cast<int>(diameter / 2);
	Image horizontalPass(image.getWidth(), image.getHeight());

	horizontalPass.initialize();

	for (std::size_t y = 0; y < image.getHeight(); y++)
	{
		for (std::size_t x = 0; x < image.getWidth(); x++)
		{
			Color result(0.0, 0.0, 0.0);

			for (int offset = -radius; offset <= radius; offset++)
			{
				const std::size_t sampleX = clampIndex(static_cast<long long>(x) + offset, image.getWidth());

				result += image.getPixel(sampleX, y) * kernel[static_cast<std::size_t>(offset + radius)];
			}
			horizontalPass.setPixel(x, y, result);
		}
	}

	for (std::size_t y = 0; y < image.getHeight(); y++)
	{
		for (std::size_t x = 0; x < image.getWidth(); x++)
		{
			Color result(0.0, 0.0, 0.0);

			for (int offset = -radius; offset <= radius; offset++)
			{
				const std::size_t sampleY = clampIndex(static_cast<long long>(y) + offset, image.getHeight());

				result += horizontalPass.getPixel(x, sampleY) * kernel[static_cast<std::size_t>(offset + radius)];
			}
			blurredImage.setPixel(x, y, result);
		}
	}
}

void	Gaussian::blur(const Image& image, Image& blurredImage)
{
	blur(image, blurredImage, 5, 1.0);
}
