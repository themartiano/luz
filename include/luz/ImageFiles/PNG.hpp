#pragma once

#include "Image.hpp"
#include <string>
#include <memory>

class	PNG
{
	public:
		PNG(void);
		PNG(std::string fileName);
		void	writeFile(const std::unique_ptr<Image>& image, bool insideDir, std::string dirName);
		void	writeFile(const std::unique_ptr<Image>& image);

	private:
		std::string	_fileName;
};
