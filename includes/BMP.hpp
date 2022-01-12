#pragma once

#include "Scene.hpp"
#include <string>

class	BMP
{
	public:
		BMP(void) = default;
		static void	writeFile(Scene& scene, bool insideDir, std::string dirName);
		static void	writeFile(Scene& scene);

	private:
		std::string	_fileName;
};
