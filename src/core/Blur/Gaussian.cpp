#include "Blur/Gaussian.hpp"
#include <cstddef>
#include <cmath>
#include <numbers>
#include <stdexcept>

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

	Kernel kernel = createKernel(diameter, amount);

	const int radius = static_cast<int>(diameter / 2);
	const std::size_t radiusSize = static_cast<std::size_t>(radius);

	blurredImage.fill(Color(0.0, 0.0, 0.0));
	if (image.getWidth() <= radiusSize * 2 || image.getHeight() <= radiusSize * 2)
	{
		return;
	}

	for (std::size_t y = radiusSize; y + radiusSize < image.getHeight(); y++)
	{
		for (std::size_t x = radiusSize; x + radiusSize < image.getWidth(); x++)
		{
			Color result(0.0, 0.0, 0.0);

			for (int bx = -radius; bx <= radius; bx++) {
				for (int by = -radius; by <= radius; by++) {
					const std::size_t sampleX = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(x) + bx);
					const std::size_t sampleY = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(y) + by);

					result += image.getPixel(sampleX, sampleY)
						* kernel[static_cast<std::size_t>(bx + radius)][static_cast<std::size_t>(by + radius)];
				}
			}

			blurredImage.setPixel(x, y, result);
		}
	}
}

void	Gaussian::blur(const Image& image, Image& blurredImage)
{
	blur(image, blurredImage, 5, 1.0);
}
