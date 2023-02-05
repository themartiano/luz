#include "Random.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <cmath>

Random randomEngine = Random();

Random::Random()
{
	this->_engine = RANDOM_ENGINE(std::random_device{}()); // Seeds
}

Random::Random(int_fast32_t seed)
{
	this->_engine = RANDOM_ENGINE(seed); // Seeds
}

void	Random::seed(int_fast32_t newSeed)
{
	this->_engine.seed(newSeed);
}

double	Random::doubleFloat(void)
{
	// std::uniform_real_distribution<double> dist;

	// return (dist(this->_engine));

	return (this->_engine() / (this->_engine.max() + 1.0));
}

double	Random::doubleFloat(double min, double max)
{
	// std::uniform_real_distribution<double> dist(min, max);

	// return (dist(this->_engine));

	return (min + (max - min) * doubleFloat());
}

unsigned int	Random::integer(void)
{
	// std::uniform_int_distribution<int> dist;

	// return (dist(this->_engine));

	return (this->_engine() / (this->_engine.max() + 1.0));
}

unsigned int	Random::integer(int min, int max)
{
	// std::uniform_int_distribution<int> dist(min, max);

	// return (dist(this->_engine));

	return (min + (max - min) * integer());
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
