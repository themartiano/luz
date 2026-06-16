#pragma once

#include "Vector3.hpp"
#include <cstddef>
#include <cstdint>

namespace Sampler
{
	struct	Sample2D
	{
		float	x = 0.0f;
		float	y = 0.0f;
	};

	enum	Dimension : std::uint32_t
	{
		DIM_CAMERA = 0,
		DIM_LENS = 1,
		DIM_LIGHT_STRATEGY = 2,
		DIM_LIGHT_SELECTION = 3,
		DIM_LIGHT_SURFACE_SELECTION = 4,
		DIM_LIGHT_SURFACE_POINT = 5,
		DIM_BSDF_DIRECTION = 6,
		DIM_MATERIAL_DECISION = 7,
		DIM_MATERIAL_FUZZ = 8,
		DIM_RUSSIAN_ROULETTE = 9,
		DIM_VOLUME_DISTANCE = 10,
		DIM_ATMOSPHERE = 11,
		DIM_ENVIRONMENT_SELECTION = 12,
		DIM_ENVIRONMENT_POINT = 13,
		DIM_ENVIRONMENT_STRATEGY = 14
	};

	void	setRenderSeed(std::uint32_t seed);
	void	beginPixelSample(std::size_t x, std::size_t y, std::uint32_t sampleIndex);
	void	beginPixelSample(std::size_t x, std::size_t y, std::uint32_t sampleIndex, std::uint32_t stream);
	void	setBounce(std::uint32_t bounce);
	void	endPixelSample(void);
	bool	isActive(void);

	double	sample1D(std::uint32_t dimension);
	Sample2D	sample2D(std::uint32_t dimension);
	Vector3	cosineHemisphere(std::uint32_t dimension);
	Vector3	sphereDirection(std::uint32_t dimension);
	Vector3	unitBall(std::uint32_t dimension);
	Vector3	unitDisk(std::uint32_t dimension);
}
