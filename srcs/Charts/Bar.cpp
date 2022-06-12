#include "Charts/Bar.hpp"

namespace Charts
{
	Bar::Bar(void)
	{
		this->_title = ""; // Should the default be empty?
	}

	void	Bar::setTitle(const std::string& title)
	{
		this->_title = title;
	}

	void	Bar::setWidth(std::size_t width)
	{
		this->_width = width;
	}

	void	Bar::setHeight(std::size_t height)
	{
		this->_height = height;
	}

	Image	Bar::generate(void)
	{
		Image	image;

		image.setWidth(this->_width);
		image.setHeight(this->_height);
		image.initialize();

		for (std::size_t y = 0; y < image.getHeight(); y++)
		{
			for (std::size_t x = 0; x < image.getWidth(); x++)
			{
				image.setPixel(x, y, Color(1.0, 1.0, 1.0));
			}
		}

		return (image);
	}
}
