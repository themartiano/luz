#include "Renderer/Renderer.hpp"
#include "Defaults.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include <thread>
#include <future>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <algorithm>

void FilterCreation(double GKernel[][5]);

void	Renderer::internal::_manageThreads(Scene& scene)
{
	static std::size_t	height = scene.getImage()->getHeight();
	static std::size_t	width = scene.getImage()->getWidth();
	static std::size_t	pixelTotal = width * height;

	volatile std::atomic<int> currentRenderPixel(0);
	std::vector<std::future<void>> futureVector;

	volatile std::atomic<double> blockRenderDifference(0.0);
	volatile std::atomic<double> oldBlockRenderTime(1.0);
	volatile std::atomic<std::size_t> blockSize(1);
	volatile std::atomic<std::size_t> oldBlockSize(1);

	// Creates threads.
	for (unsigned int i = 0; i < scene.getRenderingThreads(); i++)
	{
		futureVector.push_back(
			std::async([&scene, &currentRenderPixel, &blockRenderDifference, &blockSize, &oldBlockRenderTime, &oldBlockSize]()
			{
				Clock		pixelClock;
				Clock		blockClock;
				const bool	storePixelRenderTimes = scene.getStorePixelRenderTimes();

				std::size_t index = 0;
				while (true)
				{
					std::size_t lBlockSize = blockSize;

					std::size_t renderStopIndex = (currentRenderPixel += lBlockSize); // This way we make sure we're adding and using a 'currentRenderPixel' that has not been updated by another thread while we were doing the operations.

					blockClock.start();
					for (index = renderStopIndex - lBlockSize; index < renderStopIndex && index < pixelTotal; index++)
					{
						std::size_t	x = index % width;
						std::size_t	y = index / width;

						pixelClock.start();
						_threadRender(scene, x, y);

						if (storePixelRenderTimes)
						{
							scene.setPixelRenderTime(x, y, pixelClock.elapsedUS());
						}
					}
					double currentBlockRenderTime = blockClock.elapsedUS();

					if (index >= pixelTotal)
					{
						break;
					}

					// If it gets slower to rende, blocks will be smaller.
					blockRenderDifference = std::log1p((oldBlockRenderTime / oldBlockSize) / (currentBlockRenderTime / lBlockSize)); // The difference must be proportional to the block size.
					// std::cout << std::endl << "oldBlockRenderTime: " << oldBlockRenderTime << "; oldBlockSize: " << oldBlockSize << "; currentBlockRenderTime: " << currentBlockRenderTime << "; lBlockSize: " << lBlockSize << "; blockRenderDifference: " << blockRenderDifference << std::endl;
					oldBlockRenderTime = currentBlockRenderTime;
					oldBlockSize = lBlockSize;

					blockSize = std::min(std::max(lBlockSize * blockRenderDifference, 10.0), 1000.0);
				}
			}
		));
	}

	// Outputs progress using the main thread until the render is complete.
	while (true)
	{
		std::size_t localRenderPixel = currentRenderPixel;
		if (localRenderPixel >= pixelTotal)
		{
			break;
		}

		int percentage = (double(localRenderPixel) / double(pixelTotal)) * 100.0;
		std::cout << "\r" << CLR_CYAN << "Progress: " << CLR_WHITE << "[ " << percentage << "% ]" << std::flush;

		usleep(42 * 1000); // 42ms ~~~ (42 milliseconds * 1000 microseconds)
	}

	// Bloom
	if (true) // Will be a var later
	{
		Image brightnessImage(scene.getImage()->getWidth(), scene.getImage()->getHeight());
		brightnessImage.initialize();

		// Defines the brightness image
		for (std::size_t y = 0; y < brightnessImage.getHeight(); y++)
		{
			for (std::size_t x = 0; x < brightnessImage.getWidth(); x++)
			{
				double brightness = Utilities::luminance(scene.getImage()->getPixel(x, y));
				brightness = brightness >= 1.0 ? brightness : 0.0;
				brightnessImage.setPixel(x, y, Color(brightness, brightness, brightness));
			}
		}

		double GKernel[5][5];
		FilterCreation(GKernel);

		// Blurs the brightness image
		for (std::size_t y = 0 + 2; y < brightnessImage.getHeight() - 2; y++)
		{
			for (std::size_t x = 0 + 2; x < brightnessImage.getWidth() - 2; x++)
			{
				Color result(0.0, 0.0, 0.0);

				for (int bx = -2; bx <= 2; bx++) {
					for (int by = -2; by <= 2; by++) {
						double blurVal = GKernel[bx + 2][by + 2];

						result += brightnessImage.getPixel(x - bx, y - by) * blurVal; // R * blurVal, G * blurVal, B * blurVal
					}
				}

				brightnessImage.setPixel(x, y, result);
			}
		}

		// Adds the blurred brightness image to the original image
		for (std::size_t y = 0; y < brightnessImage.getHeight(); y++)
		{
			for (std::size_t x = 0; x < brightnessImage.getWidth(); x++)
			{
				Color result = scene.getImage()->getPixel(x, y) + brightnessImage.getPixel(x, y);
				scene.getImage()->setPixel(x, y, result);
			}
		}
	}

	if (scene.getToneMapped())
	{
		scene.getImage()->toneMap();
	}

	if (scene.getGammaCorrected())
	{
		scene.getImage()->gammaCorrect();
	}
}

void FilterCreation(double GKernel[][5])
{
	// initialising standard deviation to 1.0
	double sigma = 1.0;
	double r, s = 2.0 * sigma * sigma;

	// sum is for normalization
	double sum = 0.0;

	// generating 5x5 kernel
	for (int x = -2; x <= 2; x++) {
		for (int y = -2; y <= 2; y++) {
		   r = sqrt(x * x + y * y);
			GKernel[x + 2][y + 2] = (exp(-(r * r) / s)) / (M_PI * s);
			sum += GKernel[x + 2][y + 2];
		}
	}

	// normalising the Kernel
	for (int i = 0; i < 5; ++i)
		for (int j = 0; j < 5; ++j)
			GKernel[i][j] /= sum;
}


// Renders the pixel color at X, Y
void	Renderer::internal::_threadRender(Scene& scene, std::size_t x, std::size_t y)
{
	static int	sampleCount = scene.getSampleCount();

	Color pixelColor(0.0, 0.0, 0.0);

	for (int samples = 0; samples < sampleCount; samples++)
	{
		pixelColor += _calculatePixelColor(scene, x, y);
	}
	pixelColor /= sampleCount;

	// Replaces NaN with zeros (in case there's a problematic sample)
	if (pixelColor.getRed() != pixelColor.getRed())
	{
		pixelColor.setRed(0.0);
	}
	if (pixelColor.getGreen() != pixelColor.getGreen())
	{
		pixelColor.setGreen(0.0);
	}
	if (pixelColor.getBlue() != pixelColor.getBlue())
	{
		pixelColor.setBlue(0.0);
	}

	scene.getImage()->setPixel(x, y, pixelColor);
}
