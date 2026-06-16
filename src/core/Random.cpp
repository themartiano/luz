#include "Random.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <cmath>
#include <atomic>
#include <stdexcept>

namespace
{
	std::atomic<bool> g_hasRandomSeed(false);
	std::atomic<int_fast32_t> g_randomSeed(0);
}

thread_local Random randomEngine = Random();

Random::Random()
{
	this->_engine = RandomEngine(std::random_device{}()); // Seeds
}

Random::Random(int_fast32_t seed)
{
	this->_engine = RandomEngine(seed); // Seeds
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

	return (this->_engine());
}

unsigned int	Random::integer(int min, int max)
{
	if (min > max)
	{
		throw std::invalid_argument("Random::integer min cannot be greater than max.");
	}

	std::uniform_int_distribution<int> dist(min, max);

	return (static_cast<unsigned int>(dist(this->_engine)));
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
	const float rand1 = static_cast<float>(doubleFloat());
	const float rand2 = static_cast<float>(doubleFloat());
	const float z = std::sqrt(1.0f - rand2);

	const float phi = static_cast<float>(2.0 * D_PI) * rand1;
	const float radius = std::sqrt(rand2);
	const float x = std::cos(phi) * radius;
	const float y = std::sin(phi) * radius;

	return (Vector3(x, y, z));
}

void	setRandomSeed(int_fast32_t seed)
{
	g_randomSeed.store(seed);
	g_hasRandomSeed.store(true);
	randomEngine.seed(seed);
}

bool	hasRandomSeed(void)
{
	return (g_hasRandomSeed.load());
}

int_fast32_t	randomSeedValue(void)
{
	return (g_randomSeed.load());
}

int_fast32_t	randomSeedForThread(std::size_t threadIndex)
{
	constexpr int_fast32_t goldenRatioBits = 0x1f123bb5;

	return (g_randomSeed.load() + static_cast<int_fast32_t>((threadIndex + 1) * goldenRatioBits));
}
