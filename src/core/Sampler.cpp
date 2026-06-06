#include "Sampler.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace
{
	constexpr double	GOLDEN_RATIO_CONJUGATE = 0.6180339887498948;
	constexpr std::uint32_t	BOUNCE_DIMENSION_STRIDE = 32;
	constexpr std::uint32_t	AUXILIARY_DIMENSION_OFFSET = 0x10000u;

	struct	SampleContext
	{
		bool	active = false;
		std::size_t	x = 0;
		std::size_t	y = 0;
		std::uint32_t	sampleIndex = 0;
		std::uint32_t	stream = 0;
		std::uint32_t	bounce = 0;
	};

	std::atomic<std::uint32_t>	g_renderSeed(0x9e3779b9u);
	thread_local SampleContext	g_context;

	std::uint64_t	splitMix64(std::uint64_t value)
	{
		value += 0x9e3779b97f4a7c15ull;
		value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ull;
		value = (value ^ (value >> 27)) * 0x94d049bb133111ebull;
		return (value ^ (value >> 31));
	}

	std::uint32_t	hash32(std::uint64_t value)
	{
		return (static_cast<std::uint32_t>(splitMix64(value) >> 32));
	}

	double	unitFromHash(std::uint32_t hash)
	{
		return (static_cast<double>(hash >> 8) * (1.0 / 16777216.0));
	}

	double	unitFromHash(std::uint64_t hash)
	{
		return (static_cast<double>(hash >> 11) * (1.0 / 9007199254740992.0));
	}

	double	fract(double value)
	{
		return (value - std::floor(value));
	}

	double	dimensionStep(std::uint32_t dimension, std::uint32_t component)
	{
		const std::uint64_t key =
			(static_cast<std::uint64_t>(dimension) + 1ull) * 0x9e3779b97f4a7c15ull
			^ (static_cast<std::uint64_t>(component) + 1ull) * 0xda942042e4dd58b5ull;
		double step = unitFromHash(hash32(key));

		if (step < 0.05 || step > 0.95)
		{
			step = fract(step + GOLDEN_RATIO_CONJUGATE);
		}
		return (step);
	}

	std::uint64_t	contextKey(std::uint32_t dimension)
	{
		std::uint64_t key = g_renderSeed.load();

		key ^= (static_cast<std::uint64_t>(g_context.x) + 1ull) * 0xbf58476d1ce4e5b9ull;
		key ^= (static_cast<std::uint64_t>(g_context.y) + 1ull) * 0x94d049bb133111ebull;
		key ^= (static_cast<std::uint64_t>(g_context.stream) + 1ull) * 0xda942042e4dd58b5ull;
		key ^= (static_cast<std::uint64_t>(dimension) + 1ull) * 0x9e3779b97f4a7c15ull;
		return (key);
	}

	std::uint32_t	dimensionForBounce(std::uint32_t dimension)
	{
		return (dimension + (g_context.bounce * BOUNCE_DIMENSION_STRIDE));
	}

	bool	usesProgressiveSequence(std::uint32_t dimension)
	{
		const std::uint32_t baseDimension = dimension % BOUNCE_DIMENSION_STRIDE;

		return (
			baseDimension == Sampler::DIM_CAMERA
			|| baseDimension == Sampler::DIM_LENS
			|| baseDimension == Sampler::DIM_LIGHT_SURFACE_SELECTION
			|| baseDimension == Sampler::DIM_LIGHT_SURFACE_POINT
		);
	}

	double	hashedSample(std::uint32_t dimension, std::uint32_t component)
	{
		std::uint64_t key = contextKey(dimension);

		key ^= (static_cast<std::uint64_t>(g_context.sampleIndex) + 1ull) * 0x94d049bb133111ebull;
		key ^= (static_cast<std::uint64_t>(component) + 1ull) * 0xbf58476d1ce4e5b9ull;
		return (unitFromHash(splitMix64(key)));
	}

	Sampler::Sample2D	fallback2D(void)
	{
		return (Sampler::Sample2D{
			randomEngine.doubleFloat(),
			randomEngine.doubleFloat()
		});
	}

	double	fallback1D(void)
	{
		return (randomEngine.doubleFloat());
	}
}

void	Sampler::setRenderSeed(std::uint32_t seed)
{
	g_renderSeed.store(seed);
}

void	Sampler::beginPixelSample(std::size_t x, std::size_t y, std::uint32_t sampleIndex)
{
	beginPixelSample(x, y, sampleIndex, 0);
}

void	Sampler::beginPixelSample(std::size_t x, std::size_t y, std::uint32_t sampleIndex, std::uint32_t stream)
{
	g_context.active = true;
	g_context.x = x;
	g_context.y = y;
	g_context.sampleIndex = sampleIndex;
	g_context.stream = stream;
	g_context.bounce = 0;
}

void	Sampler::setBounce(std::uint32_t bounce)
{
	if (g_context.active)
	{
		g_context.bounce = bounce;
	}
}

void	Sampler::endPixelSample(void)
{
	g_context.active = false;
}

bool	Sampler::isActive(void)
{
	return (g_context.active);
}

double	Sampler::sample1D(std::uint32_t dimension)
{
	if (!g_context.active)
	{
		return (fallback1D());
	}

	const std::uint32_t dimensionKey = dimensionForBounce(dimension);
	if (!usesProgressiveSequence(dimension))
	{
		return (hashedSample(dimensionKey, 0));
	}
	const double shift = unitFromHash(hash32(contextKey(dimensionKey)));
	const double step = dimensionStep(dimensionKey, 0);

	return (fract(0.5 + (static_cast<double>(g_context.sampleIndex) + 1.0) * step + shift));
}

Sampler::Sample2D	Sampler::sample2D(std::uint32_t dimension)
{
	if (!g_context.active)
	{
		return (fallback2D());
	}

	const std::uint32_t dimensionKey = dimensionForBounce(dimension);
	if (!usesProgressiveSequence(dimension))
	{
		return (Sample2D{
			hashedSample(dimensionKey, 0),
			hashedSample(dimensionKey, 1)
		});
	}
	const std::uint64_t key = contextKey(dimensionKey);
	const double shiftX = unitFromHash(hash32(key));
	const double shiftY = unitFromHash(hash32(key ^ 0xda942042e4dd58b5ull));
	const double stepX = dimensionStep(dimensionKey, 0);
	const double stepY = dimensionStep(dimensionKey, 1);
	const double sample = static_cast<double>(g_context.sampleIndex) + 1.0;

	return (Sample2D{
		fract(0.5 + sample * stepX + shiftX),
		fract(0.5 + sample * stepY + shiftY)
	});
}

Vector3	Sampler::cosineHemisphere(std::uint32_t dimension)
{
	const Sample2D sample = sample2D(dimension);
	const double phi = 2.0 * D_PI * sample.x;
	const double r = std::sqrt(sample.y);
	const double z = std::sqrt(std::max(0.0, 1.0 - sample.y));

	return (Vector3(r * std::cos(phi), r * std::sin(phi), z));
}

Vector3	Sampler::sphereDirection(std::uint32_t dimension)
{
	const Sample2D sample = sample2D(dimension);
	const double z = 1.0 - (2.0 * sample.y);
	const double r = std::sqrt(std::max(0.0, 1.0 - z * z));
	const double phi = 2.0 * D_PI * sample.x;

	return (Vector3(r * std::cos(phi), r * std::sin(phi), z));
}

Vector3	Sampler::unitBall(std::uint32_t dimension)
{
	const Vector3 direction = sphereDirection(dimension);
	const double radius = std::cbrt(sample1D(dimension + AUXILIARY_DIMENSION_OFFSET));

	return (direction * radius);
}

Vector3	Sampler::unitDisk(std::uint32_t dimension)
{
	const Sample2D sample = sample2D(dimension);
	const double sx = (2.0 * sample.x) - 1.0;
	const double sy = (2.0 * sample.y) - 1.0;

	if (sx == 0.0 && sy == 0.0)
	{
		return (Vector3(0.0, 0.0, 0.0));
	}

	double radius;
	double theta;
	if (std::fabs(sx) > std::fabs(sy))
	{
		radius = sx;
		theta = (D_PI / 4.0) * (sy / sx);
	}
	else
	{
		radius = sy;
		theta = (D_PI / 2.0) - ((D_PI / 4.0) * (sx / sy));
	}

	return (Vector3(radius * std::cos(theta), radius * std::sin(theta), 0.0));
}
