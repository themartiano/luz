#pragma once

#include "Image.hpp"
#include <string>

class	BMP
{
	public:
		BMP(void);
		BMP(std::string fileName);
		void	writeFile(const std::unique_ptr<Image>& image, bool insideDir, std::string dirName);
		void	writeFile(const std::unique_ptr<Image>& image);

	private:
		std::string	_fileName;
};
