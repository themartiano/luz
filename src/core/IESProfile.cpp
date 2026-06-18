#include "IESProfile.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	const double kRadiansToDegrees = 180.0 / D_PI;

	std::string	lowerCopy(std::string value)
	{
		Utilities::toLower(value);
		return (value);
	}

	std::string	trim(const std::string& input)
	{
		std::size_t start = 0;
		std::size_t end = input.size();

		while (start < end && std::isspace(static_cast<unsigned char>(input[start])))
		{
			start++;
		}
		while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1])))
		{
			end--;
		}
		return (input.substr(start, end - start));
	}

	void	requireStrictlyIncreasing(const std::vector<double>& values, const std::string& description)
	{
		for (std::size_t i = 0; i < values.size(); i++)
		{
			if (!std::isfinite(values[i]))
			{
				throw std::runtime_error(description + " contain a non-finite value.");
			}
			if (i > 0 && values[i] <= values[i - 1])
			{
				throw std::runtime_error(description + " must be strictly increasing.");
			}
		}
	}

	std::vector<double>	readIESNumbers(std::ifstream& stream)
	{
		std::vector<double> numbers;
		std::string line;

		while (std::getline(stream, line))
		{
			std::stringstream lineStream(line);
			double value;

			while (lineStream >> value)
			{
				numbers.push_back(value);
			}
		}
		return (numbers);
	}

	std::size_t	readCount(double value, const std::string& description)
	{
		if (!std::isfinite(value) || value <= 0.0 || std::floor(value) != value)
		{
			throw std::runtime_error(description + " must be a positive integer.");
		}
		return (static_cast<std::size_t>(value));
	}

	double	normalizedHorizontalDegrees(double degrees)
	{
		if (!std::isfinite(degrees))
		{
			return (0.0);
		}
		double normalized = std::fmod(degrees, 360.0);
		if (normalized < 0.0)
		{
			normalized += 360.0;
		}
		return (normalized);
	}

	Vector3	normalizedOrDefault(Vector3 vector, Vector3 fallback)
	{
		const double lengthSquared = Utilities::vectorLengthSquared(vector);
		if (!std::isfinite(lengthSquared) || lengthSquared <= 0.0)
		{
			return (fallback);
		}
		return (vector / std::sqrt(lengthSquared));
	}

	double	interpolate(double a, double b, double t)
	{
		return (a * (1.0 - t) + b * t);
	}

	double	integrateTotalLumens(const IESProfile& profile)
	{
		constexpr int verticalSteps = 180;
		constexpr int horizontalSteps = 360;
		const double dTheta = D_PI / static_cast<double>(verticalSteps);
		const double dPhi = (2.0 * D_PI) / static_cast<double>(horizontalSteps);
		double total = 0.0;

		for (int v = 0; v < verticalSteps; v++)
		{
			const double theta = (static_cast<double>(v) + 0.5) * dTheta;
			const double verticalDegrees = theta * kRadiansToDegrees;
			const double sinTheta = std::sin(theta);

			for (int h = 0; h < horizontalSteps; h++)
			{
				const double horizontalDegrees = (static_cast<double>(h) + 0.5) * 360.0 / static_cast<double>(horizontalSteps);
				total += profile.candelaAt(verticalDegrees, horizontalDegrees) * sinTheta * dTheta * dPhi;
			}
		}
		return (total);
	}
}

IESProfile	IESProfile::load(const std::string& fileName)
{
	std::ifstream stream(fileName);

	if (!stream)
	{
		throw std::runtime_error("IES profile could not be opened: " + fileName);
	}

	std::string line;
	bool foundTilt = false;
	while (std::getline(stream, line))
	{
		const std::string cleaned = lowerCopy(trim(line));
		if (cleaned.rfind("tilt=", 0) == 0)
		{
			foundTilt = true;
			if (cleaned != "tilt=none")
			{
				throw std::runtime_error("IES profile uses unsupported tilt data: " + fileName);
			}
			break;
		}
	}
	if (!foundTilt)
	{
		throw std::runtime_error("IES profile is missing TILT=NONE: " + fileName);
	}

	const std::vector<double> numbers = readIESNumbers(stream);
	if (numbers.size() < 13)
	{
		throw std::runtime_error("IES profile has incomplete lamp metadata: " + fileName);
	}

	std::size_t index = 0;
	const double lampCount = numbers[index++];
	const double lumensPerLamp = numbers[index++];
	const double candelaMultiplier = numbers[index++];
	const std::size_t verticalCount = readCount(numbers[index++], "IES vertical angle count");
	const std::size_t horizontalCount = readCount(numbers[index++], "IES horizontal angle count");
	const double photometricType = numbers[index++];
	(void)photometricType;
	index += 7;

	const std::size_t expected = 13 + verticalCount + horizontalCount + (verticalCount * horizontalCount);
	if (numbers.size() < expected)
	{
		throw std::runtime_error("IES profile has incomplete angle or candela data: " + fileName);
	}
	if (!std::isfinite(lampCount) || lampCount <= 0.0)
	{
		throw std::runtime_error("IES profile lamp count must be finite and positive: " + fileName);
	}
	if (!std::isfinite(lumensPerLamp))
	{
		throw std::runtime_error("IES profile lumens per lamp must be finite: " + fileName);
	}
	if (!std::isfinite(candelaMultiplier) || candelaMultiplier <= 0.0)
	{
		throw std::runtime_error("IES profile candela multiplier must be finite and positive: " + fileName);
	}

	IESProfile profile;
	profile._verticalAngles.assign(numbers.begin() + index, numbers.begin() + index + verticalCount);
	index += verticalCount;
	profile._horizontalAngles.assign(numbers.begin() + index, numbers.begin() + index + horizontalCount);
	index += horizontalCount;
	profile._candelaValues.reserve(verticalCount * horizontalCount);
	for (std::size_t i = 0; i < verticalCount * horizontalCount; i++)
	{
		const double value = numbers[index + i] * candelaMultiplier;
		if (!std::isfinite(value) || value < 0.0)
		{
			throw std::runtime_error("IES profile candela values must be finite and non-negative: " + fileName);
		}
		profile._candelaValues.push_back(value);
	}

	requireStrictlyIncreasing(profile._verticalAngles, "IES vertical angles");
	requireStrictlyIncreasing(profile._horizontalAngles, "IES horizontal angles");
	profile._totalLumens = integrateTotalLumens(profile);
	if (!std::isfinite(profile._totalLumens) || profile._totalLumens <= 0.0)
	{
		throw std::runtime_error("IES profile has zero luminous flux: " + fileName);
	}
	profile._meanCandela = profile._totalLumens / (4.0 * D_PI);

	return (profile);
}

double	IESProfile::totalLumens(void) const
{
	return (this->_totalLumens);
}

double	IESProfile::candelaAt(double verticalDegrees, double horizontalDegrees) const
{
	if (
		this->_verticalAngles.empty()
		|| this->_horizontalAngles.empty()
		|| this->_candelaValues.empty()
		|| !std::isfinite(verticalDegrees)
	)
	{
		return (0.0);
	}
	if (verticalDegrees < this->_verticalAngles.front() || verticalDegrees > this->_verticalAngles.back())
	{
		return (0.0);
	}

	std::size_t v0 = 0;
	while (v0 + 1 < this->_verticalAngles.size() && this->_verticalAngles[v0 + 1] < verticalDegrees)
	{
		v0++;
	}
	const std::size_t v1 = std::min(v0 + 1, this->_verticalAngles.size() - 1);
	const double vSpan = this->_verticalAngles[v1] - this->_verticalAngles[v0];
	const double vt = vSpan > 0.0 ? (verticalDegrees - this->_verticalAngles[v0]) / vSpan : 0.0;

	std::size_t h0 = 0;
	std::size_t h1 = 0;
	double ht = 0.0;
	if (this->_horizontalAngles.size() > 1)
	{
		double horizontal = normalizedHorizontalDegrees(horizontalDegrees);
		const double first = this->_horizontalAngles.front();
		const double last = this->_horizontalAngles.back();
		if (horizontal < first)
		{
			horizontal += 360.0;
		}
		if (horizontal > last && last >= 360.0)
		{
			horizontal = std::min(horizontal, last);
		}

		while (h0 + 1 < this->_horizontalAngles.size() && this->_horizontalAngles[h0 + 1] < horizontal)
		{
			h0++;
		}
		if (h0 + 1 < this->_horizontalAngles.size())
		{
			h1 = h0 + 1;
			const double hSpan = this->_horizontalAngles[h1] - this->_horizontalAngles[h0];
			ht = hSpan > 0.0 ? (horizontal - this->_horizontalAngles[h0]) / hSpan : 0.0;
		}
		else
		{
			h1 = 0;
			const double hEnd = this->_horizontalAngles[h0];
			const double hSpan = (first + 360.0) - hEnd;
			ht = hSpan > 0.0 ? (horizontal - hEnd) / hSpan : 0.0;
		}
	}

	const std::size_t verticalCount = this->_verticalAngles.size();
	const double c00 = this->_candelaValues[h0 * verticalCount + v0];
	const double c01 = this->_candelaValues[h0 * verticalCount + v1];
	const double c10 = this->_candelaValues[h1 * verticalCount + v0];
	const double c11 = this->_candelaValues[h1 * verticalCount + v1];

	return (interpolate(interpolate(c00, c01, vt), interpolate(c10, c11, vt), ht));
}

double	IESProfile::relativeIntensity(Vector3 emissionDirection, Vector3 verticalAxis, double horizontalRotationDegrees) const
{
	if (this->_meanCandela <= 0.0 || !std::isfinite(this->_meanCandela))
	{
		return (0.0);
	}

	const Vector3 axis = normalizedOrDefault(verticalAxis, Vector3(0.0, -1.0, 0.0));
	const Vector3 direction = normalizedOrDefault(emissionDirection, axis);
	const double cosTheta = std::clamp(Utilities::dot(direction, axis), -1.0, 1.0);
	const double verticalDegrees = std::acos(cosTheta) * kRadiansToDegrees;
	const Vector3 helper = std::fabs(axis.getY()) < 0.95
		? Vector3(0.0, 1.0, 0.0)
		: Vector3(1.0, 0.0, 0.0);
	const Vector3 tangent = normalizedOrDefault(Utilities::cross(helper, axis), Vector3(1.0, 0.0, 0.0));
	const Vector3 bitangent = Utilities::cross(axis, tangent);
	const double horizontalDegrees = normalizedHorizontalDegrees(
		std::atan2(Utilities::dot(direction, bitangent), Utilities::dot(direction, tangent)) * kRadiansToDegrees
		+ horizontalRotationDegrees
	);

	return (this->candelaAt(verticalDegrees, horizontalDegrees) / this->_meanCandela);
}
