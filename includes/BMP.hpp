#ifndef BMP_HPP
#define BMP_HPP

#include "Scene.hpp"
#include <string>

class	BMP
{
	public:
		BMP(void);
		BMP(std::string fileName);
		void	writeFile(Scene& scene, double* frameBuffer);

	private:
		std::string	_fileName;
};

#endif