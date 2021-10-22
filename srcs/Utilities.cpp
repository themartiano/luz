#include "Utilities.hpp"
#include "Forms/Sphere.hpp"
#include <cmath>

// Returns the dot product of 'vector1' and 'vector2'
float	dot(Vector3 vector1, Vector3 vector2)
{
	return ((vector1.getX() * vector2.getX()) + (vector1.getY() * vector2.getY()) + (vector1.getZ() * vector2.getZ()));
}

// Returns the cross product of 'vector1' and 'vector2'
Vector3	cross(Vector3 vector1, Vector3 vector2)
{
	return (Vector3(
		(vector1.getY() * vector2.getZ()) - (vector1.getZ() * vector2.getY()),
		(vector1.getX() * vector2.getZ()) - (vector1.getZ() * vector2.getX()),
		(vector1.getX() * vector2.getY()) - (vector1.getY() * vector2.getX())));
}

// Returns the Vector3's length (no square root applied)
float	vectorLengthNoSQRT(Vector3 vector)
{
	return ((vector.getX() * vector.getX()) + (vector.getY() * vector.getY()) + (vector.getZ() * vector.getZ()));
}

// Returns the Vector3's length
float	vectorLength(Vector3 vector)
{
	return (sqrt(vectorLengthNoSQRT(vector)));
}

// Normalizes 'vector'
Vector3	normalize(Vector3 vector)
{
	vector /= vectorLength(vector);
	return (vector);
}

// Returns a 3D point (Vector3) that's random and inside a unit sphere (normalized)
Vector3 randomPointInsideUnitSphere(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(drand48(), drand48(), drand48()) * 2.0f) - Vector3(1.0f, 1.0f, 1.0f);
	} while (vectorLengthNoSQRT(position) >= 1.0f);
	return (position);
}

// Returns a 3D point (Vector3) that's random and inside a unit disk (normalized)
Vector3 randomPointInsideUnitDisk(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(drand48(), drand48(), 0) * 2.0f) - Vector3(1.0f, 1.0f, 0);
	} while (dot(position, position) >= 1.0f);
	return (position);
}

// Returns the reflected Vector3 of 'vector' using 'normal' in the calculation
Vector3	reflect(Vector3 vector, Vector3 normal)
{
	return (vector - (normal * (2.0f * dot(vector, normal))));
}

// If refraction is possible on 'vector' taking 'normal' and 'refractiveIndex' into account, sets 'refractedVector' to the new Vector3 and returns TRUE. Otherwise, returns FALSE
bool	refract(Vector3 vector, Vector3 normal, float refractiveIndex, Vector3& refractedVector)
{
	vector = normalize(vector);

	float	dt = dot(vector, normal);
	float	discriminant = 1.0f - refractiveIndex * refractiveIndex * (1.0f - dt * dt);
	if (discriminant > 0.0f)
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
float	schlick(float cosine, float refractiveIndex)
{
	float r0 = (1.0f - refractiveIndex) / (1.0f + refractiveIndex);
	r0 *= r0;

	return (r0 + (1.0f - r0) * pow((1.0f - cosine), 5));
}

// Returns "s" if 'number' differs from 1
std::string    pluralOrSingular(int number)
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
void	setFloatRange(float& flt, float min, float max)
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
