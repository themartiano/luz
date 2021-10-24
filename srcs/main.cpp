#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Forms/Sphere.hpp"
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

	scene.setXResolution(1920);
	scene.setYResolution(1080);
	scene.initializePixelArray();
	scene.setSampleCount(3);
	scene.setMaxLightBounces(6);
	scene.setGammaCorrected(true);

	// Current coordinate system ~~ Forward: -Z | Up: -Y | Right: -X
	scene.addCamera(Camera(Vector3(0.0f, 0.0f, 12.0f), Vector3(0.0f, 0.0f, -8.0f), 65, 0.3f));

	// Glass
	scene.addHittable(std::make_shared<Sphere>(Vector3(-8.0f, 0.0f, -4.0f), Material(Color(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, true), 3.0f));
	// Metal
	scene.addHittable(std::make_shared<Sphere>(Vector3(-4.0f, 0.0f, -8.0f), Material(Color(0.7f, 0.6f, 0.5f), 1.0f, 1.0f, 0.5f, 0.0f, false), 3.0f));
	// Lambertian
	scene.addHittable(std::make_shared<Sphere>(Vector3(0.0f, 0.0f, -12.0f), Material(Color(0.0f, 0.0f, 0.8f), 1.0f, 0.0f, 0.5f, 0.0f, false), 3.0f));

	// Ground (Lambertian)
	scene.addHittable(std::make_shared<Sphere>(Vector3(0.0f, 1003.0f, -8.0f), Material(Color(0.5f, 1.0f, 0.5f), 1.0f, 0.0f, 0.5f, 0.0f, false), 1000.0f));

	std::vector<std::shared_ptr<Hittable>> tinySpheres;
	for (int i = 0; i < 21; i++)
	{
		tinySpheres.push_back(std::make_shared<Sphere>(Vector3(0.0f, 0.0f, 0.0f), Material(Color(0.0f, 0.0f, 0.8f), 1.0f, 0.0f, 0.5f, 0.0f, false), 0.5f));
		//scene.addHittable(tinySpheres[i]);
	}
	scene.addHittable(std::make_shared<BVHNode>(tinySpheres));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
	(void)argv;
}
