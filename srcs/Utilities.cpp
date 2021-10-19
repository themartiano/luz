#include "Utilities.hpp"
#include "Objects/Sphere.hpp"
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
