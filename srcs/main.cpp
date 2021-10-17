#include "Scene.hpp"
#include "Exit.hpp"
#include "Luz.hpp"
#include <fstream>

int	main(int argc, char *argv[])
{
	Scene	scene;

	// if (argc <= 1)
	// 	exit_error(scene, "Scene not specified.");

	std::cout << CLR_BLUE << "Preparing...\n\n" << CLR_RESET;
	//read scene file
	//check res

	if (argc >= 3)
		//read_flags();
	(void)argv;
	init_Luz(scene);
	return (0);
}
