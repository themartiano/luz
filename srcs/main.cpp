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
	// 	exitError(scene, "Scene not specified.");

	std::cout << CLR_BLUE << "Preparing...\n\n" << CLR_RESET;
	//read scene file
	//check res

	if (argc >= 3)
	{
		//read_flags();
	}

	scene.setXResolution(1920);
	scene.setYResolution(1080);
	scene.initializePixelArray();
	scene.setSampleCount(3);
	scene.setMaxLightBounces(12);
	scene.setGammaCorrected(true);

	// Current coordinate system ~~ Forward: -Z | Up: -Y | Right: -X
	scene.addCamera(Camera(Transform(Vector3(0.0f, 0.0f, 13.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)), 65));

	scene.addSphere(Sphere(Transform(Vector3(6.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.6f, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f, 0.0f, 0.0f), 3.0f));
	//scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.5f, 0.5f, 0.5f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 3.0f));
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 1.0f, -6.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.5f, 0.5f, 0.5f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 2.0f));
	scene.addSphere(Sphere(Transform(Vector3(-6.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.5f, 0.0f), 1.0f, 1.0f, 1.0f, 0.6f, 0.0f), 3.0f));

	scene.addSphere(Sphere(Transform(Vector3(0.0f, 103.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.5f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 100.0f));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
	(void)argv;
}
