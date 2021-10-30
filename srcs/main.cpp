#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Rectangle.hpp"
#include "Forms/Plane.hpp"
#include "Material.hpp"
#include "BVHNode.hpp"
#include "Utilities.hpp"
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

	srand(time(0));

	scene.setXResolution(500);
	scene.setYResolution(500);
	scene.initializePixelArray();
	scene.setSampleCount(4);
	scene.setMaxLightBounces(2);
	scene.setGammaCorrected(true);
	scene.setRenderSky(false);
	scene.setBackgroundColor(Color(0.0f, 0.0f, 0.0f));

	// Current coordinate system ~~ Forward: -Z | Up: -Y | Right: -X

	// Cornell Box
	scene.addCamera(Camera(Vector3(0.0f, 0.0f, 390.0f), Vector3(0.0f, 0.0f, -250.0f), 65, 0.18f));


	// Top Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0f, -250.0f, -250.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
		500.0f,
		Material(Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f)));

	// Light (Top)
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0f, -250.0f, -125.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
		125.0f,
		Material(Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, true, 10.0f)));

	// Front Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0f, 0.0f, -250.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f)),
		500.0f,
		Material(Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f)));

	// Floor Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0f, 250.0f, -250.0f), Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
		500.0f,
		Material(Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f)));

	// Right Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(-250.0f, 0.0f, -250.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
		500.0f,
		Material(Color(1.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f)));

	// Left Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(250.0f, 0.0f, -250.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
		500.0f,
		Material(Color(0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f)));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
	(void)argv;
}
