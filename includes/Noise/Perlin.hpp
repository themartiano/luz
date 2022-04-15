#pragma once

#include "ImageFiles/Types.hpp"
#include <vector>
#include <string>

class	Perlin
{
	public:
		Perlin(void);
		Perlin(unsigned int seed);
		double	noise(double x, double y, double z);
		double	noise2D(double x, double y);
		void	saveToFile(std::string fileName, ImageFileTypes imageFileType, double xRes, double yRes, double noiseScale);
		void	saveToFile(ImageFileTypes imageFileType, double xRes, double yRes, double noiseScale);

	private:
		double	_fade(double t);
		double	_lerp(double t, double a, double b);
		double	_grad(int hash, double x, double y, double z);
		void	_initPermutationVector(void);

		unsigned int		_seed;
		std::vector<int>	_permutationVector;
		std::string			_defaultOutputFileName;
};
