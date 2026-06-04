#include "Scene/SceneHelpers.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include <memory>

// Dimensions and other data from https://www.graphics.cornell.edu/online/box/data.html
// Small roundings have been done
// Basic dimensions: 550w X 560d X 550h
void	SceneHelpers::cornellBox(Scene& scene, bool cubes)
{
	// Camera
	scene.addCamera(Camera(Vector3(0.0, 275.0, -1075.0), Vector3(0.0, 0.0, 1.0), 39.31, 0.0, 20.0));

	// Floor
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		550.0, 560.0,
		std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
	));

	// Ceiling
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 550.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		550.0, 560.0,
		std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
	));

	// Light (ceiling) // The order is important because the light is at the exact same position as the ceiling. Works.
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 550.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		130.0, 105.0,
		std::make_shared<Emissive>(Color(1.0, 0.84, 0.43), 29.73)
	));

	// Back wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(0.0, 275.0, 280.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
		550.0, 550.0,
		std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
	));

	// Right wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(-275.0, 275.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		560.0, 550.0,
		std::make_shared<Lambertian>(Color(0.0, 1.0, 0.0))
	));

	// Left wall
	scene.addHittable(std::make_shared<Rectangle>(
		Transform(Vector3(275.0, 275.0, 0.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		560.0, 550.0,
		std::make_shared<Lambertian>(Color(1.0, 0.0, 0.0))
	));

	if (cubes)
	{
		// // Left (back, tall) cube
		// scene.addHittable(std::make_shared<Cube>(
		// 	Transform(Vector3(0.0, 165.0, 0.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
		// 	50.0,
		// 	165.0,
		// 	50.0,
		// 	std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
		// ));

		// Right (front, short) cube
	}
}

void	SceneHelpers::cornellBox(Scene& scene)
{
	cornellBox(scene, false);
}

void	SceneHelpers::benchmark(Scene& scene)
{
	cornellBox(scene);

	// Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(225.0, 50.0, 230.0),
		50.0,
		std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.0)
	));

	// Glass Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(0.0, 150.0, 180.0),
		100.0,
		std::make_shared<Dielectric>(Color(1.0, 1.0, 1.0))
	));

	// Golden Metal Sphere
	scene.addHittable(std::make_shared<Sphere>(
		Vector3(-225.0, 50.0, 230.0),
		50.0,
		std::make_shared<Metal>(Color(1.0, 0.843, 0.0), 0.0)
	));
}
