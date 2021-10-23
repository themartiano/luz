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
	scene.setMaxLightBounces(64);
	scene.setGammaCorrected(true);

	// Current coordinate system ~~ Forward: -Z | Up: -Y | Right: -X
	scene.addCamera(Camera(Vector3(0.0f, 0.0f, 12.0f), Vector3(0.0f, 0.0f, -8.0f), 65, 0.83256987f));

	// Glass
	scene.addSphere(Sphere(Transform(Vector3(-8.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 3.0f));
	// Metal
	scene.addSphere(Sphere(Transform(Vector3(-4.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 3.0f));
	// Lambertian
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -12.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 3.0f));

	scene.addSphere(Sphere(Transform(Vector3(-3.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(2.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 0.6f));
	scene.addSphere(Sphere(Transform(Vector3(3.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(-3.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 0.47f));
	scene.addSphere(Sphere(Transform(Vector3(3.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.7f, 0.5f, 0.2f)), Material(Color(0.0f, 0.0f, 0.8f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 0.6f));

	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.6f));
	scene.addSphere(Sphere(Transform(Vector3(-1.0f, 0.0f, -12.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(-3.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.47f));
	scene.addSphere(Sphere(Transform(Vector3(4.0f, 0.0f, -12.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.6f));
	scene.addSphere(Sphere(Transform(Vector3(3.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(-3.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.7f, 0.6f, 0.5f, 0.0f), 1.0f, 1.0f, 0.5f, 0.0f, 0.0f), 0.6f));

	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -12.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 0.47f));
	scene.addSphere(Sphere(Transform(Vector3(-3.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(3.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 0.47f));
	scene.addSphere(Sphere(Transform(Vector3(6.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 0.213f));
	scene.addSphere(Sphere(Transform(Vector3(-3.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 1.0f), 0.47f));

	// Ground (Lambertian)
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 1003.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), Material(Color(0.5f, 1.0f, 0.5f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f), 1000.0f));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
	(void)argv;
}
