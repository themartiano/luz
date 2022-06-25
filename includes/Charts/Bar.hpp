#pragma once

#include "Image.hpp"
#include <string>
#include <cstdint>

namespace	Charts
{
	class	Bar
	{
		public:
			Bar(void);
			void	setTitle(const std::string& title);
			void	setWidth(std::size_t width); // Maybe the user should just set a resolution and not a width/height, which implies a certain aspect ratio?
			void	setHeight(std::size_t height);
			std::unique_ptr<Image>	generate(void);

		private:
			std::string	_title;
			std::size_t	_width;
			std::size_t	_height;
	};
}
