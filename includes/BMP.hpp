#ifndef BMP_HPP
# define BMP_HPP

#include "Scene.hpp"
#include "ANSIColors.hpp"
#include <iostream>
#include <string>

class	BMP
{
	public:
		BMP(void);
		BMP(std::string fileName);
		void	write_file(Scene scene);

	private:
		std::string	_fileName;
};

#endif