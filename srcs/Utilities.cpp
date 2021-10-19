#include "Utilities.hpp"

float	dot(Vector3 vec1, Vector3 vec2)
{
	return ((vec1.getX() * vec2.getX()) + (vec1.getY() * vec2.getY()) + (vec1.getZ() * vec2.getZ()));
}

Vector3	cross(Vector3 vec1, Vector3 vec2)
{
	return (Vector3(
		(vec1.getY() * vec2.getZ()) - (vec1.getZ() * vec2.getY()),
		(vec1.getX() * vec2.getZ()) - (vec1.getZ() * vec2.getX()),
		(vec1.getX() * vec2.getY()) - (vec1.getY() * vec2.getX())));
}

float	vectorLengthSQRT(Vector3 vector)
{
	return ((vector.getX() * vector.getX()) + (vector.getY() * vector.getY()) + (vector.getZ() * vector.getZ()));
}

float	vectorLength(Vector3 vector)
{
	return (sqrt(vectorLengthSQRT(vector)));
}

Vector3	normalize(Vector3 vector)
{
	vector /= vectorLength(vector);
	return (vector);
}

Vector3 randomPointInsideUnitSphere(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(drand48(), drand48(), drand48()) * 2.0f) - Vector3(1.0f, 1.0f, 1.0f);
	} while (vectorLengthSQRT(position) >= 1.0f);
	return (position);
}
