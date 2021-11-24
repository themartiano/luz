#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Rectangle.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Triangle.hpp"
#include "Material.hpp"
#include "BVHNode.hpp"
#include "Utilities.hpp"
#include "SystemSpecifics.hpp"
#include "SkyTypes.hpp"
#include "Defaults.hpp"
#include "OBJReader.hpp"
#include <fstream>
#include <iostream>

static void	mountCornellBox(Scene& scene);

// Main function
int	main(void)
{
	std::cout << CLR_BLUE << "Preparing...\n\n" << CLR_RESET;

	srand(time(0));

	Scene scene;
	scene.setXResolution(500);
	scene.setYResolution(500);
	scene.initializePixelArray();
	scene.setSampleCount(64);
	scene.setMaxLightBounces(12);
	scene.setGammaCorrected(true);
	scene.setRenderSky(SKY_LINEAR);
	//scene.setAtmosphere(Atmosphere(0.2, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
	//scene.setBackgroundColor(Color(0.0, 0.0, 0.0)); // Only needed if Scene.Sky == SKY_NONE

	// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

	//mountCornellBox(scene);

	scene.addCamera(Camera(Vector3(0.0, 0.5, 10.0), Vector3(0.0, 0.0, -1.0), 65, 0.0));

	std::vector<std::shared_ptr<Hittable>> triangles;

	triangles.push_back(std::make_shared<Triangle>(
		Vector3(-2.0, 0.0, 0.0),
		Vector3(2.0, 0.0, 0.0),
		Vector3(1.0, 1.0, 0.0),
		Material(Color(0.8, 1.0, 0.6), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

	scene.addHittable(std::make_shared<BVHNode>(triangles));

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
}

// Setups a Cornell Box for rendering
static void	mountCornellBox(Scene& scene)
{
	// Camera
	scene.addCamera(Camera(Vector3(0.0, 0.0, 390.0), Vector3(0.0, 0.0, 250.0), 65, 0.0));

	// Top Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -250.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Light (Top)
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -125.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
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
		Transform(Vector3(0.0, -250.0, -250.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Right Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(250.0, 0.0, -250.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(0.0, 1.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Left Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(-250.0, 0.0, -250.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		Material(Color(1.0, 0.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		500.0, 500.0
	));

	// Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(-200.0, -200.0, -200.0),
		Material(Color(0.8, 0.8, 0.8), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		50.0
	));

	// Glass Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(0.0, -100.0, -150.0),
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, true, false, 0.0),
		100.0
	));

	// Golden Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(200.0, -200.0, -200.0),
		Material(Color(1.0, 0.843, 0.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0),
		50.0
	));
}
