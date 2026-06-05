#pragma once

#include "Image.hpp"

namespace Denoise
{
	struct NonLocalMeansSettings
	{
		unsigned int	searchRadius = 3;
		unsigned int	patchRadius = 1;
		double			strength = 0.25;
	};

	void	apply(Image& image, const NonLocalMeansSettings& settings = NonLocalMeansSettings());
}
