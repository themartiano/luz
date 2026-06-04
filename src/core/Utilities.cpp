#include "Utilities.hpp"
#include "Hittables/Sphere.hpp"
#include "Defaults.hpp"
#include <cmath>
#include <algorithm>

// Returns the dot product of 'vector1' and 'vector2'
double	Utilities::dot(Vector3 vector1, Vector3 vector2)
{
	return ((vector1.getX() * vector2.getX()) + (vector1.getY() * vector2.getY()) + (vector1.getZ() * vector2.getZ()));
}

// Returns the cross product of 'vector1' and 'vector2'
Vector3	Utilities::cross(Vector3 vector1, Vector3 vector2)
{
	return (Vector3(
		(vector1.getY() * vector2.getZ()) - (vector1.getZ() * vector2.getY()),
		(vector1.getZ() * vector2.getX()) - (vector1.getX() * vector2.getZ()),
		(vector1.getX() * vector2.getY()) - (vector1.getY() * vector2.getX()))
	);
}

// Returns the Vector3's length (squared)
double	Utilities::vectorLengthSquared(Vector3 vector)
{
	return ((vector.getX() * vector.getX()) + (vector.getY() * vector.getY()) + (vector.getZ() * vector.getZ()));
}

// Returns the Vector3's length
double	Utilities::vectorLength(Vector3 vector)
{
	return (sqrt(Utilities::vectorLengthSquared(vector)));
}

// Normalizes 'vector'
Vector3	Utilities::normalize(Vector3 vector)
{
	vector /= Utilities::vectorLength(vector);
	return (vector);
}

// Returns the reflected Vector3 of 'vector' using 'normal' in the calculation
Vector3	Utilities::reflect(Vector3 vector, Vector3 normal)
{
	return (vector - 2.0 * dot(vector, normal) * normal);
}

// If refraction is possible on 'vector' taking 'normal' and 'refractiveIndex' into account, sets 'refractedVector' to the new Vector3 and returns TRUE. Otherwise, returns FALSE
Vector3	Utilities::refract(Vector3 vector, Vector3 normal, double refractiveIndex)
{
	double	cosTheta = fmin(dot(vector * -1.0, normal), 1.0);
	Vector3	perpendicularRefract = refractiveIndex * (vector + cosTheta * normal);
	Vector3	parallelRefract = -sqrt(fabs(1.0 - Utilities::vectorLengthSquared(perpendicularRefract))) * normal;

	return (perpendicularRefract + parallelRefract);
}

// Christophe Schlick's formula for approximating the contribution of the Fresnel factor in the specular reflection of light from a non-conducting interface (surface) between two media
double	Utilities::schlick(double cosine, double refractiveIndex)
{
	double r0 = (1.0 - refractiveIndex) / (1.0 + refractiveIndex);
	r0 *= r0;

	return (r0 + (1.0 - r0) * pow((1.0 - cosine), 5));
}

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

// Returns the luminance of the color 'color'
double	Utilities::luminance(const Color& color)
{
	return (
		dot(color, Vector3(0.2126, 0.7152, 0.0722))
	);
}

// Reinhard-Jodie Tone Mapping. It is applied to luminance and then scaled to RBG. In theory, at least.
Color	Utilities::reinhardJodie(const Color& color)
{
	double	l = luminance(color);
	double	max = fmax(fmax(color.getRed(), color.getGreen()), color.getBlue());

	return (Color(
		(color.getRed() * l) / max,
		(color.getGreen() * l) / max,
		(color.getBlue() * l) / max
	));
}
