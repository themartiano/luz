#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Rectangle.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Cube.hpp"
#include "Material.hpp"
#include "BVHNode.hpp"
#include "Utilities.hpp"
#include "SystemSpecifics.hpp"
#include "SkyTypes.hpp"
#include "Defaults.hpp"
#include "OBJReader.hpp"
#include "SequenceRenderer.hpp"
#include "SceneFile.hpp"
#include <fstream>
#include <iostream>
#include <vector>

static void	mountCornellBox(Scene& scene);

// Main function
int	main(int argc, char *argv[])
{
	std::cout << CLR_BLUE << "Preparing...\n\n" << CLR_RESET;

	srand(time(0));

	Scene scene;
	if (argc == 2)
	{
		readSceneFile(scene, argv[1]);
	}
	else
	{
		scene.setXResolution(500);
		scene.setYResolution(500);
		scene.setSampleCount(1);
		scene.setMaxLightBounces(4);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		//scene.setAtmosphere(Atmosphere(-0.189, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(1.0, 1.0, 1.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		//mountCornellBox(scene);

		//scene.addCamera(Camera(Vector3(0.0, D_EARTH_RADIUS + 1.0, 0.0), Vector3(0.0, 0.0, -1.0), 65, 0.0));
		scene.addCamera(Camera(Vector3(0.0, 2.5, 15.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 10.0));

		// scene.addHittable(std::make_shared<Plane>(
		// 	D_EARTH_RADIUS,
		// 	Vector3(0.0, 1.0, 0.0),
		// 	Material(Color(0.0, 1.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));

		// scene.addHittable(std::make_shared<Rectangle>(
		// 	Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
		// 	1.0,,
		// 	Material(Color(0.0, 1.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// 	1.0
		// ));

		// scene.addHittable(std::make_shared<Cube>(
		// 	Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
		// 	Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0),
		// 	1.0,
		// 	1.0,
		// 	1.0
		// ));

		// std::vector<std::shared_ptr<Hittable>> tinySpheres;
		// for (int i = 0; i < 21; i++)
		// {
		// 	tinySpheres.push_back(std::make_shared<Sphere>(Vector3(randomDouble(-5.0, 5.0), randomDouble(-5.0, 5.0), randomDouble(-5.0, 5.0)), Material(Color(randomDouble(0.0, 1.0), randomDouble(0.0, 1.0), randomDouble(0.0, 1.0)), 1.0, 0.0, 0.5, 0.0, false, false, 0.0), 0.5));
		// 	//scene.addHittable(tinySpheres[i]);
		// }
		// scene.addHittable(std::make_shared<BVHNode>(tinySpheres));

		// std::vector<std::shared_ptr<Hittable>> triangles;
		// triangles.push_back(std::make_shared<Triangle>(
		// 	Vector3(-1.0, 0.0, 0.0),
		// 	Vector3(1.0, 0.0, 0.0),
		// 	Vector3(0.0, 2.0, 0.0),
		// 	Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));
		// scene.addHittable(std::make_shared<BVHNode>(triangles));

		// scene.addHittable(std::make_shared<Triangle>(
		// 	Vector3(-1.0, 0.0, 0.0),
		// 	Vector3(1.0, 0.0, 0.0),
		//  	Vector3(0.0, 2.0, 0.0),
		// 	Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));

		//readObj(scene, "lamp.obj");
	}

	//renderSequence(scene, scene.getAtmosphere(), 5, 5.0);

	scene.updateLights();
	if (render(scene))
	{
		// Writes BMP image file
		BMP::writeFile(scene);
	}

	return (0);
}

// Setups a Cornell Box for rendering
[[maybe_unused]] static void	mountCornellBox(Scene& scene)
{
	// Camera
	scene.addCamera(Camera(Vector3(0.0, 0.0, 390.0), Vector3(0.0, 0.0, 250.0), 65, 0.0, 200.0));

	// Top Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -250.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

	// Light (Top)
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -125.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		250.0, 125.0,
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, true, 10.0)
	));

	// Back Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 0.0, -250.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

	// Floor Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, -250.0, -250.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

	// Right Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(250.0, 0.0, -250.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		Material(Color(0.0, 1.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

	// Left Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(-250.0, 0.0, -250.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		Material(Color(1.0, 0.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

	// Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(-200.0, -200.0, -200.0),
		50.0,
		Material(Color(0.8, 0.8, 0.8), 1.0, 1.0, 0.5, 0.0, false, false, 0.0)
	));

	// Glass Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(0.0, -100.0, -150.0),
		100.0,
		Material(Color(1.0, 1.0, 1.0), 1.0, 0.0, 0.5, 0.0, true, false, 0.0)
	));

	// Golden Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(200.0, -200.0, -200.0),
		50.0,
		Material(Color(1.0, 0.843, 0.0), 1.0, 1.0, 0.5, 0.0, false, false, 0.0)
	));
}
