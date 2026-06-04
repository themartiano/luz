#pragma once

#include "Image.hpp"
#include <vector>

namespace	Gaussian
{
	typedef std::vector<std::vector<double>> Kernel;

	Kernel	createKernel(unsigned int diameter, double amount);
	void	blur(const Image& image, Image& blurredImage, unsigned int diameter, double amount);
	void	blur(const Image& image, Image& blurredImage);
}
