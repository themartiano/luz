#pragma once

#include "Color.hpp"
#include <array>
#include <string>
#include <vector>

namespace MeasuredMaterials
{
	struct Conductor
	{
		Color	eta;
		Color	extinctionCoefficient;
	};

	struct Glass
	{
		double				refractiveIndexD;
		double				abbeNumber;
		bool				hasSellmeier;
		std::array<double, 3>	sellmeierB;
		std::array<double, 3>	sellmeierC;
	};

	struct Volume
	{
		Color	scatteringCoefficient;
		Color	absorptionCoefficient;
		double	anisotropy;
	};

	Conductor	conductorPreset(const std::string& name);
	Glass		glassPreset(const std::string& name);
	Volume		volumePreset(const std::string& name);
	std::string	conductorPresetNames(void);
	std::string	glassPresetNames(void);
	std::string	volumePresetNames(void);
	double		refractiveIndexFromSellmeier(
		const std::array<double, 3>& b,
		const std::array<double, 3>& c,
		double wavelengthNanometers
	);
	double		refractiveIndexFromAbbe(
		double refractiveIndexD,
		double abbeNumber,
		double wavelengthNanometers
	);
	double		volumeDensity(Color scatteringCoefficient, Color absorptionCoefficient);
	Color		volumeScatteringAlbedo(Color scatteringCoefficient, double density);
}
