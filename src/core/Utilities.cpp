#include "Utilities.hpp"
#include "Hittables/Sphere.hpp"
#include "Defaults.hpp"
#include <cmath>
#include <algorithm>
#include <filesystem>

// Returns "s" if 'number' differs from 1
std::string	Utilities::pluralOrSingular(int number)
{
	if (number == 1)
	{
		return ("");
	}
	else
	{
		return ("s");
	}
}

// If 'flt' surprasses the specified range ('min' & 'max'), it's set to the nearest possible value ('min' or 'max')
void	Utilities::setDoubleRange(double& flt, double min, double max)
{
	if (flt < min)
	{
		flt = min;
	}
	if (flt > max)
	{
		flt = max;
	}
}

// Creates the main AABB / bounding box that encapsulates all the objects inside the 'scene'
bool	Utilities::createMainBoundingBox(Scene& scene, AABB& newBoundingBox)
{
	if (scene.getHittables().size() <= 0)
	{
		return (false);
	}

	AABB	tempBB;
	bool	firstBB = true;

	for (std::shared_ptr<Hittable> hittable : scene.getHittables())
	{
		if (!hittable->createBoundingBox(tempBB))
		{
			return (false);
		}
		newBoundingBox = firstBB ? tempBB : Utilities::mergeBoundingBoxes(newBoundingBox, tempBB);
		firstBB = false;
	}

	return (true);
}

// Merges 'boundingBox1' with 'boundingBox2' and returns the resulting (and new) AABB / bounding box
AABB	Utilities::mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2)
{
	Vector3 smallestPoints(
		fmin(boundingBox1.getMinimum().getX(), boundingBox2.getMinimum().getX()),
		fmin(boundingBox1.getMinimum().getY(), boundingBox2.getMinimum().getY()),
		fmin(boundingBox1.getMinimum().getZ(), boundingBox2.getMinimum().getZ()));

	Vector3 biggestPoints(
		fmax(boundingBox1.getMaximum().getX(), boundingBox2.getMaximum().getX()),
		fmax(boundingBox1.getMaximum().getY(), boundingBox2.getMaximum().getY()),
		fmax(boundingBox1.getMaximum().getZ(), boundingBox2.getMaximum().getZ()));

	return (AABB(smallestPoints, biggestPoints));
}

// Merges all AABBs in 'boundingBoxes' and returns the resulting (and new) AABB / bounding box
AABB	Utilities::mergeBoundingBoxes(std::vector<AABB> boundingBoxes)
{
	AABB	newBoundingBox;
	bool	firstBB = true;

	for (AABB boundingBox : boundingBoxes)
	{
		newBoundingBox = firstBB ? boundingBox : Utilities::mergeBoundingBoxes(newBoundingBox, boundingBox);
		firstBB = false;
	}

	return (newBoundingBox);
}

// Converts all characters on 'str' to lower case
void	Utilities::toLower(std::string& str)
{
	std::for_each(str.begin(), str.end(), [](char& c)
	{
		c = std::tolower(c);
	});
}

// Returns true if 'str' ends with 'ending'. Otherwise, returns false
bool	Utilities::stringEndsWith(std::string str, std::string ending)
{
	if (str.size() >= ending.size())
	{
		if (str.compare(str.size() - ending.size(), ending.size(), ending) == 0)
		{
			return (true);
		}
	}

	return (false);
}

std::string	Utilities::terminalFilePath(const std::string& filePath)
{
	if (
		filePath.empty()
		|| !std::filesystem::path(filePath).is_relative()
		|| filePath.rfind("./", 0) == 0
		|| filePath.rfind(".\\", 0) == 0
		|| filePath.rfind("../", 0) == 0
		|| filePath.rfind("..\\", 0) == 0
	)
	{
		return (filePath);
	}

	return ("./" + filePath);
}

// Returns the luminance of the color 'color'
double	Utilities::luminance(const Color& color)
{
	return (
		dot(color, Vector3(0.2126, 0.7152, 0.0722))
	);
}

namespace
{
	double	clampUnit(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		if (value >= 1.0)
		{
			return (1.0);
		}
		return (value);
	}

	double	filmicToneMapChannel(double value)
	{
		constexpr double a = 2.51;
		constexpr double b = 0.03;
		constexpr double c = 2.43;
		constexpr double d = 0.59;
		constexpr double e = 0.14;

		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		return (clampUnit((value * ((a * value) + b)) / ((value * ((c * value) + d)) + e)));
	}
}

Color	Utilities::filmicToneMap(const Color& color)
{
	return (Color(
		filmicToneMapChannel(color.getRed()),
		filmicToneMapChannel(color.getGreen()),
		filmicToneMapChannel(color.getBlue())
	));
}

// Reinhard-Jodie tone mapping. Kept for callers that prefer the softer curve.
Color	Utilities::reinhardJodie(const Color& color)
{
	const double l = luminance(color);

	if (!std::isfinite(l) || l <= 0.0)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	const Color luminanceMapped = color / (1.0 + l);
	const Color channelMapped(
		color.getRed() / (1.0 + color.getRed()),
		color.getGreen() / (1.0 + color.getGreen()),
		color.getBlue() / (1.0 + color.getBlue())
	);

	return (Color(
		clampUnit((luminanceMapped.getRed() * (1.0 - channelMapped.getRed())) + (channelMapped.getRed() * channelMapped.getRed())),
		clampUnit((luminanceMapped.getGreen() * (1.0 - channelMapped.getGreen())) + (channelMapped.getGreen() * channelMapped.getGreen())),
		clampUnit((luminanceMapped.getBlue() * (1.0 - channelMapped.getBlue())) + (channelMapped.getBlue() * channelMapped.getBlue()))
	));
}
