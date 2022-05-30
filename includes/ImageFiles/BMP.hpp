#pragma once

#include "Image.hpp"
#include <string>

class	BMP
{
	public:
		BMP(void);
		BMP(std::string fileName);
		void	writeFile(const Image& image, bool insideDir, std::string dirName);
		void	writeFile(const Image& image);

	private:
		std::string	_fileName;
};
