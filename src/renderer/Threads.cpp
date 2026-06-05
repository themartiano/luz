#include "RendererInternal.hpp"
#include "Defaults.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include "Blur/Gaussian.hpp"
#include "Denoise/NonLocalMeans.hpp"
#include "Random.hpp"
#include <thread>
#include <future>
#include <vector>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <chrono>

namespace
{
	void	applyBloom(Image& image)
	{
		auto brightnessImage = image.extractBrightness();

		Gaussian::blur(*brightnessImage, *brightnessImage, 5, 1.0);
		image += *brightnessImage;
	}

	void	applyPostProcessing(Scene& scene, Image& image)
	{
		if (scene.getBloom())
		{
			applyBloom(image);
		}
		if (scene.getToneMapped())
		{
			image.toneMap();
		}
		if (scene.getGammaCorrected())
		{
			image.gammaCorrect();
		}
	}
}

void	Renderer::internal::_manageThreads(Scene& scene)
{
	const std::size_t	height = scene.getImage()->getHeight();
	const std::size_t	width = scene.getImage()->getWidth();
	const std::size_t	pixelTotal = width * height;
	const std::size_t	threadCount = std::max<std::size_t>(1, scene.getRenderingThreads());
	const std::size_t	blockSize = 16;
	const RenderCamera	renderCamera = _prepareRenderCamera(scene);

	scene.clearDenoisedImage();

	std::atomic<std::size_t> nextRenderPixel(0);
	std::atomic<std::size_t> completedRenderPixels(0);
	std::vector<std::future<void>> futureVector;
	futureVector.reserve(threadCount);

	// Creates threads.
	for (std::size_t i = 0; i < threadCount; i++)
	{
		futureVector.push_back(
			std::async(std::launch::async, [&scene, &renderCamera, &nextRenderPixel, &completedRenderPixels, width, pixelTotal, i]()
			{
				Clock		pixelClock;
				const bool	storePixelRenderTimes = scene.getStorePixelRenderTimes();

				if (hasRandomSeed())
				{
					randomEngine.seed(randomSeedForThread(i));
				}

				while (true)
				{
					const std::size_t startIndex = nextRenderPixel.fetch_add(blockSize);
					if (startIndex >= pixelTotal)
					{
						break;
					}

					const std::size_t stopIndex = std::min(startIndex + blockSize, pixelTotal);
					for (std::size_t index = startIndex; index < stopIndex; index++)
					{
						std::size_t	x = index % width;
						std::size_t	y = index / width;

						pixelClock.start();
						_threadRender(scene, renderCamera, x, y);

						if (storePixelRenderTimes)
						{
							scene.setPixelRenderTime(x, y, pixelClock.elapsedUS());
						}
					}
					completedRenderPixels.fetch_add(stopIndex - startIndex);
				}
			}
		));
	}

	// Outputs progress using the main thread until the render is complete.
	while (true)
	{
		std::size_t localRenderPixel = completedRenderPixels.load();
		if (localRenderPixel >= pixelTotal)
		{
			break;
		}

		if (!scene.getBenchmarkMode())
		{
			int percentage = (double(localRenderPixel) / double(pixelTotal)) * 100.0;
			std::cout << "\r" << CLR_CYAN << "Progress: " << CLR_WHITE << "[ " << percentage << "% ]" << std::flush;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(42));

		bool allWorkersFinished = true;
		for (std::future<void>& future : futureVector)
		{
			if (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
			{
				allWorkersFinished = false;
				break;
			}
		}
		if (allWorkersFinished)
		{
			break;
		}
	}

	for (std::future<void>& future : futureVector)
	{
		future.get();
	}

	if (scene.getDenoise())
	{
		auto denoisedImage = std::make_unique<Image>(*scene.getImage());
		Denoise::apply(*denoisedImage);
		applyPostProcessing(scene, *denoisedImage);
		scene.setDenoisedImage(std::move(denoisedImage));
	}

	applyPostProcessing(scene, *scene.getImage());
}

// Renders the pixel color at X, Y
void	Renderer::internal::_threadRender(Scene& scene, const RenderCamera& renderCamera, std::size_t x, std::size_t y)
{
	const int	sampleCount = scene.getSampleCount();

	Color pixelColor(0.0, 0.0, 0.0);

	for (int samples = 0; samples < sampleCount; samples++)
	{
		pixelColor += _calculatePixelColor(scene, renderCamera, x, y);
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
