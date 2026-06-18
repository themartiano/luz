#pragma once

#include "Vector3.hpp"
#include <string>
#include <vector>

class	IESProfile
{
	public:
		static IESProfile	load(const std::string& fileName);

		double	totalLumens(void) const;
		double	candelaAt(double verticalDegrees, double horizontalDegrees) const;
		double	relativeIntensity(Vector3 emissionDirection, Vector3 verticalAxis, double horizontalRotationDegrees) const;

	private:
		std::vector<double>	_verticalAngles;
		std::vector<double>	_horizontalAngles;
		std::vector<double>	_candelaValues;
		double				_totalLumens = 0.0;
		double				_meanCandela = 0.0;
};
