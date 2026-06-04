#include "Charts/Charts.hpp"

namespace Charts
{
	void	addText(std::unique_ptr<Image> image, const std::string& text, std::size_t x, std::size_t y, const Color& color)
	{
		std::size_t	textLength = text.length();

		// add text to image
		for (std::size_t i = 0; i < textLength; i++)
		{
			// std::size_t	character = text[i];

			for (std::size_t j = 0; j < 5; j++)
			{
				for (std::size_t k = 0; k < 5; k++)
				{
					image->setPixel(x + i * 5 + k, y + j, color);
				}
			}
		}

	}
}
