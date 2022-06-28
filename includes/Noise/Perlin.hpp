#pragma once

#include "Vector3.hpp"
#include "Image.hpp"
#include <vector>
#include <string>
#include <memory>

class	Perlin
{
	public:
		Perlin(void);
		Perlin(unsigned int seed);
		double	noise(double x, double y, double z) const;
		double	noise(const Vector3& vector) const;
		double	noise2D(double x, double y) const;
		std::unique_ptr<Image>	generateImage(double width, double height, double noiseScale) const;

	private:
		double	_fade(double t) const;
		double	_lerp(double t, double a, double b) const;
		double	_grad(int hash, double x, double y, double z) const;
		void	_initPermutationVector(void);

		unsigned int		_seed;
		std::vector<int>	_permutationVector;
		std::string			_defaultOutputFileName;
};
