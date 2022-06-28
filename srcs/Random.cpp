#include "Random.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <cmath>

void	Random::setSeed(unsigned int newSeed)
{
	seed = newSeed;
	srand(seed);
}

unsigned int	Random::getSeed(void)
{
	return (seed);
}

double	Random::doubleFloat(void)
{
	if (!isSeeded) // All other functions use this one to actually generate the random number.
	{
		// setSeed(time(NULL));
		setSeed(42);
		isSeeded = true;
	}

	return (rand() / (RAND_MAX + 1.0));
}

double	Random::doubleFloat(double min, double max)
{
	return (min + (max - min) * doubleFloat());
}

int	Random::integer(void)
{
	return (static_cast<int>(doubleFloat()));
}

int	Random::integer(int min, int max)
{
	return (static_cast<int>(doubleFloat(min, max + 1)));
}

// Returns a 3D point (Vector3) that's random and inside a unit sphere (normalized)
Vector3 Random::pointInsideUnitSphere(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(doubleFloat(), doubleFloat(), doubleFloat()) * 2.0) - Vector3(1.0, 1.0, 1.0);
	} while (Utilities::vectorLengthSquared(position) >= 1.0);

	return (position);
}

// Returns a 3D point (Vector3) that's random and inside a unit disk (normalized)
Vector3 Random::pointInsideUnitDisk(void)
{
	Vector3	position;

	do
	{
		position = (Vector3(doubleFloat(), doubleFloat(), 0) * 2.0) - Vector3(1.0, 1.0, 0);
	} while (Utilities::dot(position, position) >= 1.0);

	return (position);
}

// Generates uniform random directions
Vector3	Random::cosineDirection(void)
{
	double rand1 = doubleFloat();
	double rand2 = doubleFloat();
	double z = sqrt(1.0 - rand2);

	double phi = 2.0 * D_PI * rand1;
	double x = cos(phi) * sqrt(rand2);
	double y = sin(phi) * sqrt(rand2);

	return (Vector3(x, y, z));
}
