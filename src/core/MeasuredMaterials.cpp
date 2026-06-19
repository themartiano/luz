#include "MeasuredMaterials.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace
{
	struct NamedConductor
	{
		const char*	name;
		Color	eta;
		Color	k;
	};

	struct NamedGlass
	{
		const char*			name;
		double				nd;
		double				vd;
		bool				hasSellmeier;
		std::array<double, 3>	b;
		std::array<double, 3>	c;
	};

	struct NamedVolume
	{
		const char*	name;
		Color	sigmaS;
		Color	sigmaA;
		double	g;
	};

	std::string	normalizedName(std::string name)
	{
		Utilities::toLower(name);
		name.erase(
			std::remove_if(
				name.begin(),
				name.end(),
				[](unsigned char c)
				{
					return (c == '_' || c == '-' || std::isspace(c));
				}
			),
			name.end()
		);
		return (name);
	}

	const std::vector<NamedConductor>&	conductorTable(void)
	{
		static const std::vector<NamedConductor> table = {
			{"aluminum", Color(1.44, 0.97, 0.62), Color(7.38, 6.40, 5.30)},
			{"aluminium", Color(1.44, 0.97, 0.62), Color(7.38, 6.40, 5.30)},
			{"copper", Color(0.27, 0.68, 1.34), Color(3.57, 2.62, 2.29)},
			{"cu", Color(0.27, 0.68, 1.34), Color(3.57, 2.62, 2.29)},
			{"gold", Color(0.17, 0.47, 1.54), Color(3.14, 2.37, 1.91)},
			{"au", Color(0.17, 0.47, 1.54), Color(3.14, 2.37, 1.91)},
			{"silver", Color(0.14, 0.16, 0.13), Color(4.10, 3.14, 2.30)},
			{"ag", Color(0.14, 0.16, 0.13), Color(4.10, 3.14, 2.30)},
			{"iron", Color(2.91, 2.95, 2.82), Color(3.09, 2.93, 2.81)},
			{"fe", Color(2.91, 2.95, 2.82), Color(3.09, 2.93, 2.81)},
			{"nickel", Color(1.92, 1.83, 1.62), Color(3.67, 3.38, 3.04)},
			{"ni", Color(1.92, 1.83, 1.62), Color(3.67, 3.38, 3.04)},
			{"chromium", Color(3.21, 3.18, 2.99), Color(3.30, 3.32, 3.33)},
			{"chrome", Color(3.21, 3.18, 2.99), Color(3.30, 3.32, 3.33)}
		};

		return (table);
	}

	const std::vector<NamedGlass>&	glassTable(void)
	{
		static const std::vector<NamedGlass> table = {
			{
				"bk7",
				1.5168,
				64.17,
				true,
				{1.03961212, 0.231792344, 1.01046945},
				{0.00600069867, 0.0200179144, 103.560653}
			},
			{
				"borosilicate",
				1.474,
				65.0,
				false,
				{0.0, 0.0, 0.0},
				{0.0, 0.0, 0.0}
			},
			{
				"fusedsilica",
				1.4585,
				67.82,
				true,
				{0.6961663, 0.4079426, 0.8974794},
				{0.00467914826, 0.0135120631, 97.9340025}
			},
			{
				"silica",
				1.4585,
				67.82,
				true,
				{0.6961663, 0.4079426, 0.8974794},
				{0.00467914826, 0.0135120631, 97.9340025}
			},
			{
				"water",
				1.333,
				55.8,
				false,
				{0.0, 0.0, 0.0},
				{0.0, 0.0, 0.0}
			},
			{
				"diamond",
				2.417,
				55.3,
				false,
				{0.0, 0.0, 0.0},
				{0.0, 0.0, 0.0}
			},
			{
				"sapphire",
				1.773,
				72.2,
				false,
				{0.0, 0.0, 0.0},
				{0.0, 0.0, 0.0}
			}
		};

		return (table);
	}

	const std::vector<NamedVolume>&	volumeTable(void)
	{
		static const std::vector<NamedVolume> table = {
			{"clearair", Color(3.8e-6, 13.5e-6, 33.1e-6), Color(0.0, 0.0, 0.0), 0.0},
			{"air", Color(3.8e-6, 13.5e-6, 33.1e-6), Color(0.0, 0.0, 0.0), 0.0},
			{"haze", Color(7.5e-4, 8.5e-4, 1.0e-3), Color(1.0e-4, 1.0e-4, 1.2e-4), 0.55},
			{"mist", Color(0.025, 0.027, 0.030), Color(0.001, 0.001, 0.001), 0.72},
			{"fog", Color(0.050, 0.052, 0.055), Color(0.002, 0.002, 0.002), 0.15},
			{"smoke", Color(0.16, 0.15, 0.14), Color(0.36, 0.34, 0.32), 0.35},
			{"cloud", Color(2.0, 2.0, 2.0), Color(0.02, 0.02, 0.02), 0.85}
		};

		return (table);
	}

	template <typename T>
	std::string	namesFromTable(const std::vector<T>& table)
	{
		std::string names;

		for (const T& item : table)
		{
			if (!names.empty())
			{
				names += ", ";
			}
			names += item.name;
		}
		return (names);
	}

	void	requireFinitePositive(double value, const std::string& label)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			throw std::invalid_argument(label + " must be finite and positive.");
		}
	}

	void	requireFiniteNonNegativeColor(Color color, const std::string& label)
	{
		if (
			!std::isfinite(color.getRed()) || color.getRed() < 0.0
			|| !std::isfinite(color.getGreen()) || color.getGreen() < 0.0
			|| !std::isfinite(color.getBlue()) || color.getBlue() < 0.0
		)
		{
			throw std::invalid_argument(label + " must have finite non-negative channels.");
		}
	}
}

MeasuredMaterials::Conductor	MeasuredMaterials::conductorPreset(const std::string& name)
{
	const std::string normalized = normalizedName(name);

	for (const NamedConductor& preset : conductorTable())
	{
		if (normalizedName(preset.name) == normalized)
		{
			return (Conductor{preset.eta, preset.k});
		}
	}
	throw std::invalid_argument("Unknown conductor preset '" + name + "'. Available presets: " + conductorPresetNames() + ".");
}

MeasuredMaterials::Glass	MeasuredMaterials::glassPreset(const std::string& name)
{
	const std::string normalized = normalizedName(name);

	for (const NamedGlass& preset : glassTable())
	{
		if (normalizedName(preset.name) == normalized)
		{
			return (Glass{
				preset.nd,
				preset.vd,
				preset.hasSellmeier,
				preset.b,
				preset.c
			});
		}
	}
	throw std::invalid_argument("Unknown glass preset '" + name + "'. Available presets: " + glassPresetNames() + ".");
}

MeasuredMaterials::Volume	MeasuredMaterials::volumePreset(const std::string& name)
{
	const std::string normalized = normalizedName(name);

	for (const NamedVolume& preset : volumeTable())
	{
		if (normalizedName(preset.name) == normalized)
		{
			return (Volume{preset.sigmaS, preset.sigmaA, preset.g});
		}
	}
	throw std::invalid_argument("Unknown volume preset '" + name + "'. Available presets: " + volumePresetNames() + ".");
}

std::string	MeasuredMaterials::conductorPresetNames(void)
{
	return (namesFromTable(conductorTable()));
}

std::string	MeasuredMaterials::glassPresetNames(void)
{
	return (namesFromTable(glassTable()));
}

std::string	MeasuredMaterials::volumePresetNames(void)
{
	return (namesFromTable(volumeTable()));
}

double	MeasuredMaterials::refractiveIndexFromSellmeier(
	const std::array<double, 3>& b,
	const std::array<double, 3>& c,
	double wavelengthNanometers
)
{
	requireFinitePositive(wavelengthNanometers, "Sellmeier wavelength");
	const double wavelengthMicrometers = wavelengthNanometers * 0.001;
	const double lambdaSquared = wavelengthMicrometers * wavelengthMicrometers;
	double nSquared = 1.0;

	for (std::size_t i = 0; i < 3; i++)
	{
		if (!std::isfinite(b[i]) || !std::isfinite(c[i]) || c[i] < 0.0)
		{
			throw std::invalid_argument("Sellmeier coefficients must be finite, with non-negative C terms.");
		}
		const double denominator = lambdaSquared - c[i];
		if (std::fabs(denominator) <= 1e-12)
		{
			throw std::invalid_argument("Sellmeier wavelength hits a resonance denominator.");
		}
		nSquared += (b[i] * lambdaSquared) / denominator;
	}
	if (!std::isfinite(nSquared) || nSquared <= 0.0)
	{
		throw std::invalid_argument("Sellmeier coefficients produced an invalid refractive index.");
	}
	return (std::sqrt(nSquared));
}

double	MeasuredMaterials::refractiveIndexFromAbbe(
	double refractiveIndexD,
	double abbeNumber,
	double wavelengthNanometers
)
{
	requireFinitePositive(refractiveIndexD, "Abbe refractive index");
	requireFinitePositive(abbeNumber, "Abbe number");
	requireFinitePositive(wavelengthNanometers, "Abbe wavelength");

	constexpr double lambdaF = 486.1327;
	constexpr double lambdaD = 587.5618;
	constexpr double lambdaC = 656.2725;
	const double dispersion = (refractiveIndexD - 1.0) / abbeNumber;
	const double b = dispersion
		/ ((1.0 / (lambdaF * lambdaF)) - (1.0 / (lambdaC * lambdaC)));
	const double a = refractiveIndexD - (b / (lambdaD * lambdaD));

	return (a + (b / (wavelengthNanometers * wavelengthNanometers)));
}

double	MeasuredMaterials::volumeDensity(Color scatteringCoefficient, Color absorptionCoefficient)
{
	requireFiniteNonNegativeColor(scatteringCoefficient, "Volume scattering coefficient");
	requireFiniteNonNegativeColor(absorptionCoefficient, "Volume absorption coefficient");

	const Color extinction = scatteringCoefficient + absorptionCoefficient;
	const double density = Utilities::luminance(extinction);
	if (!std::isfinite(density) || density <= 0.0)
	{
		throw std::invalid_argument("Measured volume coefficients produce zero extinction.");
	}
	return (density);
}

Color	MeasuredMaterials::volumeScatteringAlbedo(Color scatteringCoefficient, double density)
{
	requireFiniteNonNegativeColor(scatteringCoefficient, "Volume scattering coefficient");
	requireFinitePositive(density, "Volume density");

	return (Color(
		std::min(1.0, scatteringCoefficient.getRed() / density),
		std::min(1.0, scatteringCoefficient.getGreen() / density),
		std::min(1.0, scatteringCoefficient.getBlue() / density)
	));
}
