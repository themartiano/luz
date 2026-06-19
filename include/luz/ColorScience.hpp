#pragma once

#include "Color.hpp"
#include <string>
#include <vector>

namespace ColorScience
{
	struct SpectralSample
	{
		double	wavelengthNanometers;
		double	value;
	};

	Color	wavelength(double nanometers);
	Color	blackbody(double kelvin);
	Color	solar(void);
	Color	reflectanceCurve(std::vector<SpectralSample> samples);
	Color	loadReflectanceCurve(const std::string& fileName);
}
