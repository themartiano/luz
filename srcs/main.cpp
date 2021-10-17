#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Luz.hpp"
#include "Renderer.hpp"
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
	init_Luz(scene);

	render(scene);

	// Writes BMP image file
	BMP bmp("render.bmp");
	bmp.write_file(scene);

	return (0);
	(void)argv;
}
