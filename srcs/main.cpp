#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include <fstream>
#include <iostream>

// Main function
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

	scene.setXResolution(1920);
	scene.setYResolution(1080);

	// Current coordinate system ~~ Forward: -Z | Up: -Y
	scene.addCamera(Camera(Transform(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)), 70));
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.6f, 0.6f, 0.6f, 0.0f), 1.0f), 4.0f));
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 104.0f, -10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.5f, 0.0f, 0.0f, 0.0f), 1.0f), 100.0f));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.write_file(scene);

	return (0);
	(void)argv;
}
