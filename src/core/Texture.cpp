#include "Texture.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
	std::string	readPPMToken(std::istream& stream)
	{
		std::string token;
		char c;

		while (stream.get(c))
		{
			if (std::isspace(static_cast<unsigned char>(c)))
			{
				continue;
			}
			if (c == '#')
			{
				stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				continue;
			}
			token.push_back(c);
			break;
		}
		while (stream.get(c))
		{
			if (std::isspace(static_cast<unsigned char>(c)))
			{
				break;
			}
			token.push_back(c);
		}
		return (token);
	}

	double	clamp01(double value)
	{
		return (std::max(0.0, std::min(1.0, value)));
	}

	double	wrap01(double value)
	{
		value = value - std::floor(value);
		if (value < 0.0)
		{
			value += 1.0;
		}
		return (value);
	}
}

Texture::Texture(void)
{
	this->_width = 0;
	this->_height = 0;
}

Texture::Texture(std::size_t width, std::size_t height, std::vector<Color> pixels)
{
	this->_width = width;
	this->_height = height;
	this->_pixels = std::move(pixels);
}

Texture	Texture::loadPPM(const std::string& fileName)
{
	std::ifstream stream(fileName, std::ios::binary);

	if (!stream)
	{
		throw std::runtime_error("Texture file could not be opened: " + fileName);
	}

	const std::string magic = readPPMToken(stream);
	if (magic != "P6" && magic != "P3")
	{
		throw std::runtime_error("Unsupported texture format. Use binary or ASCII PPM: " + fileName);
	}

	const std::size_t width = static_cast<std::size_t>(std::stoul(readPPMToken(stream)));
	const std::size_t height = static_cast<std::size_t>(std::stoul(readPPMToken(stream)));
	const double maxValue = std::stod(readPPMToken(stream));
	if (width == 0 || height == 0 || maxValue <= 0.0)
	{
		throw std::runtime_error("Invalid PPM texture header: " + fileName);
	}

	std::vector<Color> pixels;
	pixels.reserve(width * height);
	if (magic == "P6")
	{
		for (std::size_t i = 0; i < width * height; i++)
		{
			unsigned char rgb[3];
			if (!stream.read(reinterpret_cast<char*>(rgb), 3))
			{
				throw std::runtime_error("Truncated PPM texture: " + fileName);
			}
			pixels.emplace_back(
				static_cast<double>(rgb[0]) / maxValue,
				static_cast<double>(rgb[1]) / maxValue,
				static_cast<double>(rgb[2]) / maxValue
			);
		}
	}
	else
	{
		for (std::size_t i = 0; i < width * height; i++)
		{
			pixels.emplace_back(
				std::stod(readPPMToken(stream)) / maxValue,
				std::stod(readPPMToken(stream)) / maxValue,
				std::stod(readPPMToken(stream)) / maxValue
			);
		}
	}

	return (Texture(width, height, std::move(pixels)));
}

Color	Texture::sample(double u, double v) const
{
	if (this->empty())
	{
		return (Color(1.0, 1.0, 1.0));
	}

	u = wrap01(u);
	v = wrap01(v);
	const double x = u * static_cast<double>(this->_width - 1);
	const double y = (1.0 - v) * static_cast<double>(this->_height - 1);
	const std::size_t x0 = static_cast<std::size_t>(std::floor(x));
	const std::size_t y0 = static_cast<std::size_t>(std::floor(y));
	const std::size_t x1 = std::min(x0 + 1, this->_width - 1);
	const std::size_t y1 = std::min(y0 + 1, this->_height - 1);
	const double tx = x - static_cast<double>(x0);
	const double ty = y - static_cast<double>(y0);

	const Color c00 = this->_pixels[y0 * this->_width + x0];
	const Color c10 = this->_pixels[y0 * this->_width + x1];
	const Color c01 = this->_pixels[y1 * this->_width + x0];
	const Color c11 = this->_pixels[y1 * this->_width + x1];
	const Color top = c00 * (1.0 - tx) + c10 * tx;
	const Color bottom = c01 * (1.0 - tx) + c11 * tx;

	return (Color(
		clamp01((top * (1.0 - ty) + bottom * ty).getRed()),
		clamp01((top * (1.0 - ty) + bottom * ty).getGreen()),
		clamp01((top * (1.0 - ty) + bottom * ty).getBlue())
	));
}

std::size_t	Texture::getWidth(void) const
{
	return (this->_width);
}

std::size_t	Texture::getHeight(void) const
{
	return (this->_height);
}

bool	Texture::empty(void) const
{
	return (this->_width == 0 || this->_height == 0 || this->_pixels.empty());
}
