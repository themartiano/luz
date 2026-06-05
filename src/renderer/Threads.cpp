#include "RendererInternal.hpp"
#include "Defaults.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include "Blur/Gaussian.hpp"
#include "Denoise/NFOR.hpp"
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
	struct	DenoiseHalfAccumulator
	{
		Color	colorSum;
		Denoise::FeatureVector	featureSum;
		unsigned int	count = 0;
	};

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

	void	printRenderProgress(unsigned int percentage)
	{
		std::cout
			<< "\r" << CLR_CYAN << "Rendering: "
			<< CLR_WHITE << "[ " << percentage << "% ]"
			<< CLR_RESET << std::flush;
	}

	void	printDenoiseProgress(unsigned int percentage, void*)
	{
		std::cout
			<< "\r" << CLR_CYAN << "Denoising: "
			<< CLR_WHITE << "[ " << percentage << "% ]"
			<< CLR_RESET << std::flush;
	}

	void	printSkippingDenoising(void)
	{
		std::cout << CLR_YELLOW << "Skipping denoising." << CLR_RESET << std::endl;
	}

	Color	cleanColor(Color color)
	{
		if (!std::isfinite(color.getRed()))
		{
			color.setRed(0.0);
		}
		if (!std::isfinite(color.getGreen()))
		{
			color.setGreen(0.0);
		}
		if (!std::isfinite(color.getBlue()))
		{
			color.setBlue(0.0);
		}
		return (color);
	}

	void	addDenoiseFeatureSample(
		DenoiseHalfAccumulator& half,
		Denoise::FeatureVector& featureSum,
		Denoise::FeatureVector& featureSquareSum,
		const Denoise::FeatureVector& feature
	)
	{
		for (std::size_t i = 0; i < Denoise::NFOR_FEATURE_COUNT; i++)
		{
			const double value = feature[i];

			half.featureSum[i] += value;
			featureSum[i] += value;
			featureSquareSum[i] += value * value;
		}
	}

	Denoise::FeatureVector	meanFeature(
		const Denoise::FeatureVector& sum,
		unsigned int count,
		const Denoise::FeatureVector& fallback
	)
	{
		if (count == 0)
		{
			return (fallback);
		}

		Denoise::FeatureVector result;
		for (std::size_t i = 0; i < Denoise::NFOR_FEATURE_COUNT; i++)
		{
			result[i] = sum[i] / static_cast<double>(count);
		}
		return (result);
	}

	double	sampleMeanVariance(double sum, double squareSum, unsigned int count)
	{
		if (count <= 1)
		{
			return (0.0);
		}

		const double n = static_cast<double>(count);
		const double variance = (squareSum - (sum * sum / n)) / (n - 1.0);

		return (std::max(0.0, variance / n));
	}

	double	colorVariance(Color sum, Color squareSum, unsigned int count)
	{
		return (
			sampleMeanVariance(sum.getRed(), squareSum.getRed(), count)
			+ sampleMeanVariance(sum.getGreen(), squareSum.getGreen(), count)
			+ sampleMeanVariance(sum.getBlue(), squareSum.getBlue(), count)
		) / 3.0;
	}

	Denoise::FeatureVector	featureVariance(
		const Denoise::FeatureVector& sum,
		const Denoise::FeatureVector& squareSum,
		unsigned int count
	)
	{
		Denoise::FeatureVector result;

		for (std::size_t i = 0; i < Denoise::NFOR_FEATURE_COUNT; i++)
		{
			result[i] = sampleMeanVariance(sum[i], squareSum[i], count);
		}
		return (result);
	}

	Color	meanColor(Color sum, unsigned int count, Color fallback)
	{
		if (count == 0)
		{
			return (fallback);
		}
		return (sum / static_cast<double>(count));
	}

	void	storeDenoisePixel(
		Scene& scene,
		std::size_t x,
		std::size_t y,
		const DenoiseHalfAccumulator& halfA,
		const DenoiseHalfAccumulator& halfB,
		Color colorSum,
		Color colorSquareSum,
		const Denoise::FeatureVector& featureSum,
		const Denoise::FeatureVector& featureSquareSum,
		unsigned int sampleCount
	)
	{
		Denoise::NFORBuffers* buffers = scene.getDenoiseBuffers();
		if (buffers == nullptr)
		{
			return;
		}

		const std::size_t index = buffers->index(x, y);
		const Color fallbackColor = meanColor(colorSum, sampleCount, Color());
		const Denoise::FeatureVector fallbackFeature = meanFeature(featureSum, sampleCount, Denoise::FeatureVector());

		buffers->colorA[index] = meanColor(halfA.colorSum, halfA.count, fallbackColor);
		buffers->colorB[index] = meanColor(halfB.colorSum, halfB.count, fallbackColor);
		buffers->colorVariance[index] = colorVariance(colorSum, colorSquareSum, sampleCount);
		buffers->featuresA[index] = meanFeature(halfA.featureSum, halfA.count, fallbackFeature);
		buffers->featuresB[index] = meanFeature(halfB.featureSum, halfB.count, fallbackFeature);
		buffers->featureVariance[index] = featureVariance(featureSum, featureSquareSum, sampleCount);
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
	if (scene.getDenoise())
	{
		scene.initializeDenoiseBuffers(width, height);
	}
	else
	{
		scene.clearDenoiseBuffers();
	}

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
			printRenderProgress(static_cast<unsigned int>(percentage));
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

	if (!scene.getBenchmarkMode())
	{
		printRenderProgress(100);
		std::cout << std::endl;
	}

	if (scene.getDenoise() && scene.getDenoiseBuffers() != nullptr)
	{
		Denoise::NFORSettings nforSettings;

		nforSettings.threadCount = scene.getRenderingThreads();
		if (!scene.getBenchmarkMode())
		{
			nforSettings.progressCallback = printDenoiseProgress;
		}
		auto denoisedImage = Denoise::applyNFOR(*scene.getDenoiseBuffers(), nforSettings);
		if (!scene.getBenchmarkMode())
		{
			std::cout << std::endl;
		}
		applyPostProcessing(scene, *denoisedImage);
		scene.setDenoisedImage(std::move(denoisedImage));
		scene.clearDenoiseBuffers();
	}
	else if (!scene.getBenchmarkMode())
	{
		printSkippingDenoising();
	}

	applyPostProcessing(scene, *scene.getImage());
}

// Renders the pixel color at X, Y
void	Renderer::internal::_threadRender(Scene& scene, const RenderCamera& renderCamera, std::size_t x, std::size_t y)
{
	const int	sampleCount = scene.getSampleCount();
	Denoise::NFORBuffers* denoiseBuffers = scene.getDenoiseBuffers();

	if (denoiseBuffers == nullptr)
	{
		Color pixelColor(0.0, 0.0, 0.0);

		for (int samples = 0; samples < sampleCount; samples++)
		{
			pixelColor += cleanColor(_calculatePixelColor(scene, renderCamera, x, y));
		}
		pixelColor /= sampleCount;
		scene.getImage()->setPixel(x, y, cleanColor(pixelColor));
		return;
	}

	Color pixelColor(0.0, 0.0, 0.0);
	Color pixelColorSquare(0.0, 0.0, 0.0);
	Denoise::FeatureVector featureSum;
	Denoise::FeatureVector featureSquareSum;
	DenoiseHalfAccumulator halfA;
	DenoiseHalfAccumulator halfB;

	for (int samples = 0; samples < sampleCount; samples++)
	{
		RenderSample sample = _calculatePixelSample(scene, renderCamera, x, y);
		DenoiseHalfAccumulator& half = (samples % 2 == 0) ? halfA : halfB;
		Color sampleColor = cleanColor(sample.color);

		half.colorSum += sampleColor;
		half.count++;
		addDenoiseFeatureSample(half, featureSum, featureSquareSum, sample.features);
		pixelColor += sampleColor;
		pixelColorSquare += Color(
			sampleColor.getRed() * sampleColor.getRed(),
			sampleColor.getGreen() * sampleColor.getGreen(),
			sampleColor.getBlue() * sampleColor.getBlue()
		);
	}
	const Color pixelColorSum = pixelColor;
	pixelColor /= sampleCount;
	pixelColor = cleanColor(pixelColor);
	storeDenoisePixel(
		scene,
		x,
		y,
		halfA,
		halfB,
		pixelColorSum,
		pixelColorSquare,
		featureSum,
		featureSquareSum,
		static_cast<unsigned int>(sampleCount)
	);

	scene.getImage()->setPixel(x, y, pixelColor);
}
