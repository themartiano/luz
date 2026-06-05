#include "Scene/SceneHelpers.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Mesh.hpp"
#include "Hittables/Triangle.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Atmosphere.hpp"
#include "SkyTypes.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	void	addBenchmarkRoom(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 2.2, -7.0), Vector3(0.0, -0.18, 1.0), 45.0, 0.0, 8.0));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			12.0,
			12.0,
			std::make_shared<Lambertian>(Color(0.72, 0.72, 0.72))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 6.0, 1.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			5.0,
			5.0,
			std::make_shared<Emissive>(Color(1.0, 0.9, 0.72), 12.0)
		));
	}

	void	benchmarkDefault(Scene& scene)
	{
		SceneHelpers::cornellBox(scene);

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

	void	benchmarkManyObjects(Scene& scene)
	{
		addBenchmarkRoom(scene);

		const auto red = std::make_shared<Lambertian>(Color(0.82, 0.25, 0.2));
		const auto blue = std::make_shared<Lambertian>(Color(0.22, 0.42, 0.86));
		const auto green = std::make_shared<Lambertian>(Color(0.25, 0.72, 0.34));
		const auto metal = std::make_shared<Metal>(Color(0.75, 0.72, 0.66), 0.15);

		for (int z = 0; z < 15; z++)
		{
			for (int x = 0; x < 15; x++)
			{
				std::shared_ptr<Material> material = red;
				if ((x + z) % 5 == 0)
				{
					material = metal;
				}
				else if ((x + z) % 3 == 0)
				{
					material = blue;
				}
				else if ((x + z) % 2 == 0)
				{
					material = green;
				}

				scene.addHittable(std::make_shared<Sphere>(
					Vector3((x - 7) * 0.62, 0.18, z * 0.45 - 0.5),
					0.18,
					material
				));
			}
		}
	}

	void	benchmarkMeshBVH(Scene& scene)
	{
		addBenchmarkRoom(scene);

		std::vector<std::shared_ptr<Hittable>> triangles;
		const auto material = std::make_shared<Lambertian>(Color(0.7, 0.72, 0.78));
		const int gridSize = 44;
		const double step = 0.14;
		const double start = -gridSize * step * 0.5;

		triangles.reserve(gridSize * gridSize * 2);
		for (int y = 0; y < gridSize; y++)
		{
			for (int x = 0; x < gridSize; x++)
			{
				const double x0 = start + x * step;
				const double x1 = x0 + step;
				const double y0 = 0.6 + y * step;
				const double y1 = y0 + step;
				const double z00 = 1.2 + 0.08 * ((x + y) % 3);
				const double z10 = 1.2 + 0.08 * ((x + 1 + y) % 3);
				const double z01 = 1.2 + 0.08 * ((x + y + 1) % 3);
				const double z11 = 1.2 + 0.08 * ((x + y + 2) % 3);

				triangles.push_back(std::make_shared<Triangle>(
					Vector3(x0, y0, z00),
					Vector3(x1, y0, z10),
					Vector3(x0, y1, z01),
					material
				));
				triangles.push_back(std::make_shared<Triangle>(
					Vector3(x1, y0, z10),
					Vector3(x1, y1, z11),
					Vector3(x0, y1, z01),
					material
				));
			}
		}

		scene.addHittable(std::make_shared<Mesh>(
			Vector3(),
			material,
			triangles
		));
	}

	void	benchmarkDiffuse(Scene& scene)
	{
		addBenchmarkRoom(scene);

		const auto white = std::make_shared<Lambertian>(Color(0.82, 0.82, 0.78));
		const auto warm = std::make_shared<Lambertian>(Color(0.95, 0.54, 0.28));
		const auto cool = std::make_shared<Lambertian>(Color(0.32, 0.57, 0.9));

		for (int z = 0; z < 8; z++)
		{
			for (int x = 0; x < 8; x++)
			{
				const double radius = 0.26 + 0.04 * ((x + z) % 3);
				std::shared_ptr<Material> material = white;
				if ((x + z) % 3 == 1)
				{
					material = warm;
				}
				else if ((x + z) % 3 == 2)
				{
					material = cool;
				}
				scene.addHittable(std::make_shared<Sphere>(
					Vector3((x - 3.5) * 0.72, radius, z * 0.62 - 0.3),
					radius,
					material
				));
			}
		}
	}

	void	benchmarkPostProcess(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 1.7, -5.5), Vector3(0.0, -0.12, 1.0), 48.0, 0.0, 6.0));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 0.0, 0.5), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			8.0,
			8.0,
			std::make_shared<Lambertian>(Color(0.65, 0.65, 0.68))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 2.8, 1.4), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			4.5,
			2.5,
			std::make_shared<Emissive>(Color(1.0, 0.93, 0.72), 24.0)
		));
	}

	void	benchmarkAtmosphere(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 6360120.0, 600.0), Vector3(0.0, 0.0, -1.0), 42.0, 0.0, 200.0));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 6360000.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			1200.0,
			1200.0,
			std::make_shared<Lambertian>(Color(0.78, 0.78, 0.72))
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-110.0, 6360055.0, 60.0),
			42.0,
			std::make_shared<Emissive>(Color(1.0, 1.0, 0.63), 5.0)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(150.0, 6360075.0, -95.0),
			75.0,
			std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.0)
		));
		scene.setRenderSky(SKY_ATMOSPHERE);
		scene.setAtmosphere(Atmosphere(0.3, 6360000.0, 6420000.0, 7994.0, 1200.0, 16, 8, 0.5));
	}
}

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
	benchmark(scene, "default");
}

void	SceneHelpers::benchmark(Scene& scene, const std::string& benchmarkCase)
{
	if (benchmarkCase == "default")
	{
		benchmarkDefault(scene);
	}
	else if (benchmarkCase == "many-objects")
	{
		benchmarkManyObjects(scene);
	}
	else if (benchmarkCase == "mesh-bvh")
	{
		benchmarkMeshBVH(scene);
	}
	else if (benchmarkCase == "diffuse")
	{
		benchmarkDiffuse(scene);
	}
	else if (benchmarkCase == "postprocess")
	{
		benchmarkPostProcess(scene);
	}
	else if (benchmarkCase == "atmosphere")
	{
		benchmarkAtmosphere(scene);
	}
	else
	{
		throw std::runtime_error("Unknown benchmark case: " + benchmarkCase);
	}
}
