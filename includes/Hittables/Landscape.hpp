#pragma once

#include "Hittables/Procedural.hpp"

class	Landscape : public Procedural
{
	public:
		Landscape(void);
		Landscape(Vector3 position, double size, Color color, unsigned int subSamples, double noiseScale, double magnitude, double depth, unsigned int seed);

	private:
		unsigned int	_seed;
};
