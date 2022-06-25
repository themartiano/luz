#include "Noise/Perlin.hpp"
#include "Scene.hpp"
#include <numeric>
#include <random>
#include <algorithm>

Perlin::Perlin(void)
{
	this->_seed = 42;
	this->_defaultOutputFileName = "perlinNoise";

	this->_initPermutationVector();
}

Perlin::Perlin(unsigned int seed)
{
	this->_seed = seed;
	this->_defaultOutputFileName = "perlinNoise";

	this->_initPermutationVector();
}

double	Perlin::_fade(double t) const
{
	return (t * t * t * (t * (t * 6.0 - 15.0) + 10.0));
}

double	Perlin::_lerp(double t, double a, double b) const
{
	return (a + t * (b - a));
}

double	Perlin::_grad(int hash, double x, double y, double z) const
{
	int h = hash & 15;

	double u = h < 8 ? x : y;
	double v = h < 4 ? y : h == 12 || h == 14 ? x : z;

	return (((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v));
}

void Perlin::_initPermutationVector(void)
{
	this->_permutationVector.resize(256);

	// Fills the vector with values ranging from 0 to (size - 1, 255 in our case)
	std::iota(this->_permutationVector.begin(), this->_permutationVector.end(), 0);

	std::default_random_engine engine(this->_seed);
	std::shuffle(this->_permutationVector.begin(), this->_permutationVector.end(), engine);

	//duplicate?
}

double Perlin::noise(double x, double y, double z) const
{
	int X = (int)std::floor(x) & 255;
	int Y = (int)std::floor(y) & 255;
	int Z = (int)std::floor(z) & 255;

	x -= std::floor(X);
	y -= std::floor(Y);
	z -= std::floor(Z);

	double u = this->_fade(x);
	double v = this->_fade(y);
	double w = this->_fade(z);

	int A = this->_permutationVector[X] + Y;
	int AA = this->_permutationVector[A] + Z;
	int AB = this->_permutationVector[A + 1] + Z;
	int B = this->_permutationVector[X + 1] + Y;
	int BA = this->_permutationVector[B] + Z;
	int BB = this->_permutationVector[B + 1] + Z;

	double output = this->_lerp(
		w,
		this->_lerp(
			v,
			this->_lerp(
				u,
				this->_grad(this->_permutationVector[AA], x, y, z),
				this->_grad(this->_permutationVector[BA], x - 1, y, z)
			),
			this->_lerp(
				u,
				this->_grad(this->_permutationVector[AB], x, y - 1, z),
				this->_grad(this->_permutationVector[BB], x - 1, y - 1, z)
			)
		),
		this->_lerp(
			v,
			this->_lerp(
				u,
				this->_grad(this->_permutationVector[AA + 1], x, y, z - 1),
				this->_grad(this->_permutationVector[BA + 1], x - 1, y, z - 1)
			),
			this->_lerp(
				u,
				this->_grad(this->_permutationVector[AB + 1], x, y - 1, z - 1),
				this->_grad(this->_permutationVector[BB + 1], x - 1, y - 1, z - 1)
			)
		)
	);

	return ((output + 1.0) / 2.0);
}

double	Perlin::noise(const Vector3& vector) const
{
	return (this->noise(vector.getX(), vector.getY(), vector.getZ()));
}

// 2D noise can have any Z value
double Perlin::noise2D(double x, double y) const
{
	return (this->noise(x, y, 0.0));
}

std::unique_ptr<Image>	Perlin::generateImage(double width, double height, double noiseScale) const
{
	auto image = std::make_unique<Image>(width, height);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			double y = (double)i / ((double)height);
			double x = (double)j / ((double)width);

			double n = this->noise2D(noiseScale * x, noiseScale * y);

			image->setPixel(j, i, Color(n, n, n));
		}
	}

	return (image);
}
