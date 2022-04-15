#pragma once

#include "ImageFiles/Types.hpp"
#include <vector>
#include <string>

class	Perlin
{
	public:
		Perlin(void);
		Perlin(unsigned int seed);
		double	noise(double x, double y, double z) const;
		double	noise2D(double x, double y) const;
		void	saveToFile(std::string fileName, ImageFileTypes imageFileType, double xRes, double yRes, double noiseScale) const;
		void	saveToFile(ImageFileTypes imageFileType, double xRes, double yRes, double noiseScale) const;

	private:
		double	_fade(double t) const;
		double	_lerp(double t, double a, double b) const;
		double	_grad(int hash, double x, double y, double z) const;
		void	_initPermutationVector(void);

		unsigned int		_seed;
		std::vector<int>	_permutationVector;
		std::string			_defaultOutputFileName;
};
