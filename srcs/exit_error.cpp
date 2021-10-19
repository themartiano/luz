#include "Exit.hpp"
#include "ANSIColors.hpp"
#include <iostream>

// Prints 'message' as an error and exits with code 1
void	exit_error(Scene &scene, std::string message)
{
	std::cout << CLR_RED << "Error\n" << CLR_RESET;
	std::cout << CLR_RED_BRIGHT << message << "\n" << CLR_RESET;
	//clean_exit(scene, 1);
	exit(1);
	(void)scene;
}
