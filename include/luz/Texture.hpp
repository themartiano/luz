#pragma once

#include "Color.hpp"
#include "ColorManagement.hpp"
#include <cstddef>
#include <string>
#include <vector>

class	Texture
{
	public:
		Texture(void);
		Texture(std::size_t width, std::size_t height, std::vector<Color> pixels);
		static Texture	loadPPM(const std::string& fileName, TextureColorRole colorRole = TextureColorRole::ColorSRGB);
		Color	sample(double u, double v) const;
		std::size_t	getWidth(void) const;
		std::size_t	getHeight(void) const;
		bool	empty(void) const;

	private:
		std::size_t	_width;
		std::size_t	_height;
		std::vector<Color>	_pixels;
};
