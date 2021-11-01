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
#include "SystemSpecifics.hpp"
#include <fstream>
#include <iostream>

static void	mountCornellBox(Scene& scene);

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
	scene.setMaxLightBounces(4);
	scene.setGammaCorrected(true);
	scene.setRenderSky(true);
	scene.setBackgroundColor(Color(0.0, 0.0, 0.0));

	// Current coordinate system ~~ Forward: -Z | Up: -Y | Right: -X

	//mountCornellBox(scene);
	scene.addCamera(Camera(Vector3(0.0, -5.0, 0.0), Vector3(0.0, -5.0, -1.0), 65, 0.0));

	// scene.addHittable(std::make_shared<Plane>(
	// 	0.0,
	// 	Vector3(0.0, -1.0, 0.0),
	// 	Material(Color(0.33, 0.49, 0.27), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	// ));

	scene.addHittable(std::make_shared<Sphere>(
		Vector3(0.0, -3.0, -20.0),
		//Material(Color(0.8, 0.8, 0.8), 1.0, 0.0, 0.5, 0.0, false, true, 5.0),
		Material(Color(1.0, 0.0, 0.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		3.0
	));
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(0.0, -3.0, 20.0),
		//Material(Color(0.8, 0.8, 0.8), 1.0, 0.0, 0.5, 0.0, false, true, 5.0),
		Material(Color(0.0, 1.0, 0.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		3.0
	));
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(-20.0, -3.0, 0),
		//Material(Color(0.8, 0.8, 0.8), 1.0, 0.0, 0.5, 0.0, false, true, 5.0),
		Material(Color(0.0, 0.0, 1.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		3.0
	));
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(20.0, -3.0, 0.0),
		//Material(Color(0.8, 0.8, 0.8), 1.0, 0.0, 0.5, 0.0, false, true, 5.0),
		Material(Color(0.0, 1.0, 1.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		3.0
	));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
	(void)argv;
}

// Setups a Cornell Box for rendering
static void	mountCornellBox(Scene& scene)
{
	// Camera
	scene.addCamera(Camera(Vector3(0.0, 0.0, 390.0), Vector3(0.0, 0.0, -250.0), 65, 0.0));

	// Top Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, -250.0, -250.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Light (Top)
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, -250.0, -125.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, true, 10.0),
		250.0, 125.0
	));

	// Back Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 0.0, -250.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Floor Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -250.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Right Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(-250.0, 0.0, -250.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(0.0, 1.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Left Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(250.0, 0.0, -250.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 0.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(200.0, 200.0, -200.0),
		Material(Color(0.8, 0.8, 0.8), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		50.0
	));

	// Glass Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(0.0, 100.0, -150.0),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, true, false, 0.0),
		100.0
	));

	// Golden Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(-200.0, 200.0, -200.0),
		Material(Color(1.0, 0.843, 0.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		50.0
	));
}
