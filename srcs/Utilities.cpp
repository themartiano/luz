#include "Utilities.hpp"
#include "Forms/Sphere.hpp"
#include <cmath>
#include <algorithm>

// Returns the dot product of 'vector1' and 'vector2'
double	Utilities::Utilities::dot(Vector3 vector1, Vector3 vector2)
{
	return ((vector1.getX() * vector2.getX()) + (vector1.getY() * vector2.getY()) + (vector1.getZ() * vector2.getZ()));
}

// Returns the cross product of 'vector1' and 'vector2'
Vector3	Utilities::Utilities::cross(Vector3 vector1, Vector3 vector2)
{
	return (Vector3(
		(vector1.getY() * vector2.getZ()) - (vector1.getZ() * vector2.getY()),
		(vector1.getX() * vector2.getZ()) - (vector1.getZ() * vector2.getX()),
		(vector1.getX() * vector2.getY()) - (vector1.getY() * vector2.getX())));
}

// Returns the Vector3's length (no square root applied)
double	Utilities::Utilities::vectorLengthNoSQRT(Vector3 vector)
{
	return ((vector.getX() * vector.getX()) + (vector.getY() * vector.getY()) + (vector.getZ() * vector.getZ()));
}

// Returns the Vector3's length
double	Utilities::Utilities::vectorLength(Vector3 vector)
{
	return (sqrt(Utilities::vectorLengthNoSQRT(vector)));
}

// Normalizes 'vector'
Vector3	Utilities::Utilities::normalize(Vector3 vector)
{
	vector /= Utilities::vectorLength(vector);
	return (vector);
}

// Returns a 3D point (Vector3) that's random and inside a unit sphere (normalized)
Vector3 Utilities::Utilities::randomPointInsideUnitSphere(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(randomDouble(), randomDouble(), randomDouble()) * 2.0) - Vector3(1.0, 1.0, 1.0);
	} while (Utilities::vectorLengthNoSQRT(position) >= 1.0);
	return (position);
}

// Returns a 3D point (Vector3) that's random and inside a unit disk (normalized)
Vector3 Utilities::Utilities::randomPointInsideUnitDisk(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(randomDouble(), randomDouble(), 0) * 2.0) - Vector3(1.0, 1.0, 0);
	} while (Utilities::dot(position, position) >= 1.0);
	return (position);
}

// Returns the reflected Vector3 of 'vector' using 'normal' in the calculation
Vector3	Utilities::Utilities::reflect(Vector3 vector, Vector3 normal)
{
	return (vector - (normal * (2.0 * Utilities::dot(vector, normal))));
}

// If refraction is possible on 'vector' taking 'normal' and 'refractiveIndex' into account, sets 'refractedVector' to the new Vector3 and returns TRUE. Otherwise, returns FALSE
bool	Utilities::Utilities::refract(Vector3 vector, Vector3 normal, double refractiveIndex, Vector3& refractedVector)
{
	vector = Utilities::normalize(vector);

	double	dt = Utilities::dot(vector, normal);
	double	discriminant = 1.0 - refractiveIndex * refractiveIndex * (1.0 - dt * dt);
	if (discriminant > 0.0)
	{
		refractedVector = ((vector - normal * dt) * refractiveIndex) - normal * sqrt(discriminant);
		return (true);
	}
	else
	{
		return (false);
	}
}

// Christophe Schlick's formula for approximating the contribution of the Fresnel factor in the specular reflection of light from a non-conducting interface (surface) between two media
double	Utilities::Utilities::schlick(double cosine, double refractiveIndex)
{
	double r0 = (1.0 - refractiveIndex) / (1.0 + refractiveIndex);
	r0 *= r0;

	return (r0 + (1.0 - r0) * pow((1.0 - cosine), 5));
}

// Returns "s" if 'number' differs from 1
std::string    Utilities::Utilities::pluralOrSingular(int number)
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
void	Utilities::Utilities::setDoubleRange(double& flt, double min, double max)
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
bool	Utilities::Utilities::createMainBoundingBox(Scene& scene, AABB& newBoundingBox)
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
AABB	Utilities::Utilities::mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2)
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

void	Utilities::toLower(std::string& str)
{
	std::for_each(str.begin(), str.end(), [](char& c)
	{
		c = std::tolower(c);
	});
}
