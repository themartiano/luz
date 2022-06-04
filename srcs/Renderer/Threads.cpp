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
	static int	height = scene.getImageHeight();
	static int	width = scene.getImageWidth();
	static int	pixelTotal = width * height;

	static unsigned int	threadCount = CORE_COUNT * THREAD_MULTIPLIER;
	volatile std::atomic<int> currentRenderPixel(0);
	std::vector<std::future<void>> futureVector;

	volatile std::atomic<double> blockRenderDifference(0.0);
	volatile std::atomic<int> blockSize(1);

	// Creates threads.
	for (unsigned int i = 0; i < threadCount; i++)
	{
		futureVector.push_back(
			std::async([&scene, &currentRenderPixel, &blockRenderDifference, &blockSize]()
			{
				Clock		clock(false);
				const bool	storePixelRenderTimes = scene.getStorePixelRenderTimes();
				double		blockRenderTime = 1.0;

				int index = 0;
				while (true)
				{
					if (index >= pixelTotal)
					{
						break;
					}

					int newBlockSize;
					blockSize = newBlockSize = std::min(std::max(blockRenderDifference * 10.0 * blockSize, 1.0), 100.0);

					int renderStartIndex = currentRenderPixel;
					currentRenderPixel += newBlockSize;

					for (index = renderStartIndex; index < renderStartIndex + newBlockSize && index < pixelTotal; index++)
					{
						int	x = index % width;
						int	y = index / width;

						clock.start();
						_threadRender(scene, x, y);

						if (storePixelRenderTimes)
						{
							scene.setPixelRenderTime(x, y, clock.elapsed(false));
						}
					}

					double currentBlockRenderTime = clock.stop(false) + 1.0;
					blockRenderDifference = std::log1p(blockRenderTime / currentBlockRenderTime);
					blockRenderTime = currentBlockRenderTime;
				}
			}
		));
	}

	// Outputs progress using the main thread until the render is complete.
	while (true)
	{
		int localRenderPixel = currentRenderPixel;
		if (localRenderPixel >= pixelTotal)
		{
			break;
		}

		int percentage = (double(localRenderPixel) / double(pixelTotal)) * 100.0;
		std::cout << "\r" << CLR_CYAN << "Progress: " << CLR_WHITE << "[ " << percentage << "% ]" << std::flush;

		usleep(42 * 1000); // 42ms ~~~ (42 milliseconds * 1000 microseconds)
	}
}

// Renders the pixel color at X, Y
void	Renderer::internal::_threadRender(Scene& scene, int x, int y)
{
	static int	sampleCount = scene.getSampleCount();
	static bool	gammaCorrected = scene.getGammaCorrected();

	Color pixelColor(0.0, 0.0, 0.0);

	for (int samples = 0; samples < sampleCount; samples++)
	{
		pixelColor += _calculatePixelColor(scene, x, y);
	}
	pixelColor /= sampleCount;
	if (gammaCorrected)
	{
		pixelColor = Color(sqrtf(pixelColor.getRed()), sqrtf(pixelColor.getGreen()), sqrtf(pixelColor.getBlue())); // Gamma (2) correction
	}

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

	scene.getImage().setPixel(x, y, pixelColor);
}
