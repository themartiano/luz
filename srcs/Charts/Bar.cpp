#include "Charts/Bar.hpp"
#include "Charts/Charts.hpp"
#include <utility>

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

	std::unique_ptr<Image>	Bar::generate(void)
	{
		auto image = std::make_unique<Image>(this->_width, this->_height);

		image->initialize();
		image->fill(Color(1.0, 1.0, 1.0));

		addText(std::move(image), this->_title, 0, 0, Color(0.0, 0.0, 0.0)); // This has to be fixed

		return (image);
	}
}
