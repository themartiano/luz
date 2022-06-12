#include "Charts/Bar.hpp"
#include "Charts/Charts.hpp"

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
		image.fill(Color(1.0, 1.0, 1.0));

		addText(image, this->_title, 0, 0, Color(0.0, 0.0, 0.0));

		return (image);
	}
}
