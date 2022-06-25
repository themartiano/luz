#pragma once

#include "Image.hpp"

namespace	Charts
{
	void	addText(std::unique_ptr<Image> image, const std::string& text, std::size_t x, std::size_t y, const Color& color);
}
