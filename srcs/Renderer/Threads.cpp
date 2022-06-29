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

void	Renderer::internal::_manageThreads(Scene& scene)
{
	static std::size_t	height = scene.getImage()->getHeight();
	static std::size_t	width = scene.getImage()->getWidth();
	static std::size_t	pixelTotal = width * height;

	static unsigned int	threadCount = CORE_COUNT * THREAD_MULTIPLIER;
	volatile std::atomic<int> currentRenderPixel(0);
	std::vector<std::future<void>> futureVector;

	volatile std::atomic<double> blockRenderDifference(0.0);
	volatile std::atomic<double> oldBlockRenderTime(1.0);
	volatile std::atomic<std::size_t> blockSize(1);
	volatile std::atomic<std::size_t> oldBlockSize(1);

	// Creates threads.
	for (unsigned int i = 0; i < threadCount; i++)
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

	if (scene.getToneMapped())
	{
		scene.getImage()->toneMap();
	}

	if (scene.getGammaCorrected())
	{
		scene.getImage()->gammaCorrect();
	}
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
