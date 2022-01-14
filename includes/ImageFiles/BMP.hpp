#pragma once

#include "Scene.hpp"
#include <string>

class	BMP
{
	public:
		BMP(void);
		BMP(std::string fileName);
		void	writeFile(Scene& scene, bool insideDir, std::string dirName);
		void	writeFile(Scene& scene);

	private:
		std::string	_fileName;
};
