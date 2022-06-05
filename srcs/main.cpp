#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile/SceneFile.hpp"
#include "OBJReader.hpp"
#include "Hittables/Cloud.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/PerlinSphere.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Mesh.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/ConstantVolume.hpp"
#include "Hittables/Landscape.hpp"
#include "Hittables/WaterBody.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Isotropic.hpp"
#include "Noise/Perlin.hpp"

static void	mountCornellBox(Scene& scene);

// Main function
int	main(int argc, char *argv[])
{
	std::cout << CLR_BLUE << "Preparing..." << CLR_RESET << std::endl << std::endl;

	Scene scene;
	scene.setStorePixelRenderTimes(true); // Test so it works for scene files (before it's natively there)
	if (argc == 2)
	{
		SceneFile::read(scene, argv[1]);
	}
	else
	{
		scene.setImageHeight(500);
		scene.setImageWidth(500);
		scene.setSampleCount(10);
		scene.setMaxLightBounces(50);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		// scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(0.0, 0.0, 0.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		mountCornellBox(scene);

		scene.addHittable(std::make_shared<Mesh>(readObj("objects/blender_monkey.obj", Vector3(0.0, -25.0, -100.0), std::make_shared<Dielectric>(Color(0.42, 0.42, 0.42)))));

		// Metal Sphere
		// scene.addHittable(std::make_shared<Sphere>(
		// 	Vector3(-200.0, -200.0, -200.0),
		// 	50.0,
		// 	std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.0)
		// ));

		// Glass Sphere
		// scene.addHittable(std::make_shared<Sphere>(
		// 	Vector3(0.0, -100.0, -150.0),
		// 	100.0,
		// 	std::make_shared<Dielectric>(Color(1.0, 1.0, 1.0)) // check that
		// ));

		// Golden Metal Sphere
		// scene.addHittable(std::make_shared<Sphere>(
		// 	Vector3(200.0, -200.0, -200.0),
		// 	50.0,
		// 	std::make_shared<Metal>(Color(1.0, 0.843, 0.0), 0.0)
		// ));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.getImage().saveToBMP("render");
		Image debugTime = scene.generateRenderTimeImage();
		debugTime.saveToBMP("renderTime");
	}

	return (0);
}

// Setups a Cornell Box for rendering
[[maybe_unused]] static void	mountCornellBox(Scene& scene)
{
	// Camera
	scene.addCamera(Camera(Vector3(0.0, 0.0, 390.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 20.0));

	// Top Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -250.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
	));

	// Light (Top)
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 250.0, -125.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		250.0, 125.0,
		std::make_shared<Emissive>(Color(1.0, 1.0, 1.0), 10.0)
	));

	// Back Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 0.0, -250.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
	));

	// Floor Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, -250.0, -250.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
	));

	// Right Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(250.0, 0.0, -250.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		std::make_shared<Lambertian>(Color(0.0, 1.0, 0.0))
	));

	// Left Wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(-250.0, 0.0, -250.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		500.0, 500.0,
		std::make_shared<Lambertian>(Color(1.0, 0.0, 0.0))
	));
}
