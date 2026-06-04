#include "Exit.hpp"
#include "ANSIColors.hpp"
#include <iostream>

// Prints 'message' as an error and exits with code (1)
void	exitError(Scene &scene, std::string message)
{
	std::cerr << CLR_RED << "Error\n" << CLR_RESET;
	std::cerr << CLR_RED_BRIGHT << message << "\n" << CLR_RESET;
	//clean_exit(scene, 1);
	exit(1);
	(void)scene;
}
