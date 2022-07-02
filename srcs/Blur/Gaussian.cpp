#include "Blur/Gaussian.hpp"
#include <cmath>

Gaussian::Kernel	Gaussian::createKernel(unsigned int diameter, double amount)
{
	double radius = (diameter / 2);

	Kernel kernel;

	double r;
	double s = 2.0 * amount * amount;
	double sum = 0.0;

	kernel.resize(diameter);
	for (int x = -radius; x <= radius; x++)
	{
		kernel[x + radius].resize(diameter);

		for (int y = -radius; y <= radius; y++)
		{
			r = sqrt(x * x + y * y);
			kernel[x + radius][y + radius] = (exp(-(r * r) / s)) / (M_PI * s);
			sum += kernel[x + radius][y + radius];
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
	Kernel kernel = createKernel(diameter, amount);

	double radius = (diameter / 2);
	for (std::size_t y = radius; y < image.getHeight() - radius; y++)
	{
		for (std::size_t x = radius; x < image.getWidth() - radius; x++)
		{
			Color result(0.0, 0.0, 0.0);

			for (int bx = -radius; bx <= radius; bx++) {
				for (int by = -radius; by <= radius; by++) {
					result += image.getPixel(x - bx, y - by) * kernel[bx + radius][by + radius]; // R * blurVal, G * blurVal, B * blurVal
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
