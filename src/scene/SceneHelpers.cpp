#include "Scene/SceneHelpers.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Mesh.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/ConstantVolume.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Isotropic.hpp"
#include "Materials/HenyeyGreenstein.hpp"
#include "Materials/Principled.hpp"
#include "Atmosphere.hpp"
#include "AssetPath.hpp"
#include "OBJReader.hpp"
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

	void	addBenchmarkBackdrop(Scene& scene)
	{
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 0.0, 1.4), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			10.0,
			12.0,
			std::make_shared<Lambertian>(Color(0.72, 0.72, 0.68))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 2.6, 6.2), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			10.0,
			5.2,
			std::make_shared<Lambertian>(Color(0.68, 0.70, 0.76))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(-5.0, 2.6, 1.4), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			12.0,
			5.2,
			std::make_shared<Lambertian>(Color(0.42, 0.58, 0.52))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(5.0, 2.6, 1.4), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			12.0,
			5.2,
			std::make_shared<Lambertian>(Color(0.72, 0.46, 0.42))
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

	void	benchmarkLights(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 2.4, -7.3), Vector3(0.0, -0.18, 1.0), 46.0, 0.0, 8.0));
		addBenchmarkBackdrop(scene);

		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 5.1, 1.2), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			3.4,
			2.2,
			std::make_shared<Emissive>(Color(1.0, 0.86, 0.62), 9.0)
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(-4.7, 2.7, 2.8), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			2.6,
			2.8,
			std::make_shared<Emissive>(Color(0.45, 0.62, 1.0), 3.5)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-1.75, 1.2, -0.3),
			0.12,
			std::make_shared<Emissive>(Color(1.0, 0.35, 0.24), 10.0)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(2.1, 1.0, 2.4),
			0.16,
			std::make_shared<Emissive>(Color(0.34, 0.9, 1.0), 7.0)
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-1.4, 0.55, 1.2),
			0.55,
			std::make_shared<Dielectric>(Color(1.0, 1.0, 1.0))
		));
		scene.addHittable(std::make_shared<Cube>(
			Transform(Vector3(1.25, 0.52, 1.1), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			0.9,
			1.04,
			0.9,
			std::make_shared<Metal>(Color(0.82, 0.78, 0.68), 0.2)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(0.1, 0.34, 3.0),
			0.34,
			std::make_shared<Lambertian>(Color(0.86, 0.38, 0.24))
		));
	}

	void	benchmarkEmissiveGeometry(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 2.2, -6.6), Vector3(0.0, -0.16, 1.0), 48.0, 0.0, 7.0));
		addBenchmarkBackdrop(scene);

		const auto meshLight = std::make_shared<Emissive>(Color(1.0, 0.74, 0.38), 7.5);
		std::vector<std::shared_ptr<Hittable>> lightTriangles;

		lightTriangles.push_back(std::make_shared<Triangle>(
			Vector3(-1.8, 3.7, 1.2),
			Vector3(0.0, 4.8, 1.4),
			Vector3(0.0, 3.35, 2.9),
			meshLight
		));
		lightTriangles.push_back(std::make_shared<Triangle>(
			Vector3(1.8, 3.7, 1.2),
			Vector3(0.0, 3.35, 2.9),
			Vector3(0.0, 4.8, 1.4),
			meshLight
		));
		scene.addHittable(std::make_shared<Mesh>(
			Vector3(),
			meshLight,
			lightTriangles
		));
		scene.addHittable(std::make_shared<Triangle>(
			Vector3(-2.6, 0.12, 2.8),
			Vector3(-1.7, 1.55, 2.4),
			Vector3(-1.1, 0.12, 3.6),
			std::make_shared<Emissive>(Color(0.38, 0.78, 1.0), 4.2)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(2.15, 0.45, 2.0),
			0.45,
			std::make_shared<Emissive>(Color(1.0, 0.28, 0.48), 4.8)
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-0.95, 0.5, 1.35),
			0.5,
			std::make_shared<Metal>(Color(0.82, 0.82, 0.78), 0.08)
		));
		scene.addHittable(std::make_shared<Cube>(
			Transform(Vector3(0.8, 0.42, 2.85), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			0.84,
			0.84,
			0.84,
			std::make_shared<Lambertian>(Color(0.78, 0.55, 0.36))
		));
	}

	void	benchmarkPrimitivesMaterials(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 2.0, -7.0), Vector3(0.0, -0.12, 1.0), 47.0, 0.08, 7.0));

		scene.addHittable(std::make_shared<Plane>(
			0.0,
			Vector3(0.0, 1.0, 0.0),
			std::make_shared<Lambertian>(Color(0.7, 0.7, 0.66))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 2.5, 6.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			9.0,
			5.0,
			std::make_shared<Lambertian>(Color(0.62, 0.68, 0.76))
		));
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 4.8, 1.2), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			3.2,
			2.2,
			std::make_shared<Emissive>(Color(1.0, 0.88, 0.7), 8.0)
		));
		scene.addHittable(std::make_shared<Cube>(
			Transform(Vector3(-2.0, 0.55, 1.7), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			0.9,
			1.1,
			0.9,
			std::make_shared<Principled>(Color(0.65, 0.34, 0.24), 0.0, 0.38)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-0.55, 0.65, 2.45),
			0.65,
			std::make_shared<Metal>(Color(0.82, 0.78, 0.7), 0.18)
		));

		const auto glassBoundary = std::make_shared<Sphere>(
			Vector3(1.1, 0.68, 1.25),
			0.68,
			std::make_shared<Dielectric>(Color(0.9, 1.0, 1.0))
		);
		scene.addHittable(glassBoundary);
		scene.addHittable(std::make_shared<ConstantVolume>(
			glassBoundary,
			std::make_shared<Isotropic>(Color(0.42, 0.62, 0.9)),
			0.28
		));
		scene.addHittable(std::make_shared<Triangle>(
			Vector3(1.95, 0.05, 2.9),
			Vector3(2.8, 1.45, 2.65),
			Vector3(3.2, 0.05, 3.75),
			std::make_shared<Lambertian>(Color(0.28, 0.62, 0.46))
		));
	}

	void	benchmarkVolumes(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 2.15, -7.2), Vector3(0.0, -0.13, 1.0), 47.0, 0.0, 7.0));
		addBenchmarkBackdrop(scene);

		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 5.1, 1.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			3.6,
			2.4,
			std::make_shared<Emissive>(Color(1.0, 0.86, 0.68), 9.5)
		));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(2.4, 1.8, 4.5),
			0.22,
			std::make_shared<Emissive>(Color(0.55, 0.72, 1.0), 12.0)
		));

		const auto glassBoundary = std::make_shared<Sphere>(
			Vector3(-1.15, 0.95, 1.8),
			0.95,
			std::make_shared<Dielectric>(Color(0.9, 0.96, 1.0))
		);
		scene.addHittable(glassBoundary);
		scene.addHittable(std::make_shared<ConstantVolume>(
			glassBoundary,
			std::make_shared<Isotropic>(Color(0.38, 0.55, 0.95)),
			0.42
		));

		const auto smokeBoundary = std::make_shared<Cube>(
			Transform(Vector3(1.35, 0.8, 2.25), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			1.35,
			1.6,
			1.35,
			std::make_shared<Lambertian>(Color(0.18, 0.18, 0.18))
		);
		scene.addHittable(std::make_shared<ConstantVolume>(
			smokeBoundary,
			std::make_shared<HenyeyGreenstein>(Color(0.7, 0.68, 0.62), 0.35),
			0.65
		));

		const auto mistBoundary = std::make_shared<Sphere>(
			Vector3(0.15, 0.55, 3.35),
			0.55,
			std::make_shared<Lambertian>(Color(0.42, 0.42, 0.42))
		);
		scene.addHittable(std::make_shared<ConstantVolume>(
			mistBoundary,
			std::make_shared<Isotropic>(Color(0.62, 0.78, 0.7)),
			0.9
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-2.45, 0.35, 3.4),
			0.35,
			std::make_shared<Metal>(Color(0.78, 0.76, 0.7), 0.12)
		));
		scene.addHittable(std::make_shared<Triangle>(
			Vector3(2.0, 0.05, 0.8),
			Vector3(2.7, 1.25, 1.2),
			Vector3(3.15, 0.05, 1.9),
			std::make_shared<Lambertian>(Color(0.55, 0.36, 0.28))
		));
	}

	void	benchmarkObjMesh(Scene& scene)
	{
		scene.addCamera(Camera(Vector3(0.0, 2.2, -7.2), Vector3(0.0, -0.13, 1.0), 45.0, 0.0, 7.0));
		addBenchmarkBackdrop(scene);
		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 5.0, 1.2), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			3.4,
			2.0,
			std::make_shared<Emissive>(Color(1.0, 0.9, 0.74), 9.5)
		));

		ObjReadOptions options;
		options.quiet = true;
		const std::string suzannePath = AssetPath::resolve("suzanne.obj");

		scene.addHittable(std::make_shared<Mesh>(readObj(
			suzannePath,
			Vector3(-0.8, 0.05, 1.1),
			Vector3(0.0, 180.0, 0.0),
			Vector3(1.35, 1.35, 1.35),
			std::make_shared<Principled>(Color(0.62, 0.64, 0.68), 0.15, 0.42),
			options
		)));
		scene.addHittable(std::make_shared<Mesh>(readObj(
			suzannePath,
			Vector3(1.05, 0.0, 1.6),
			Vector3(0.0, 35.0, 0.0),
			Vector3(0.95, 0.95, 0.95),
			std::make_shared<Metal>(Color(0.78, 0.72, 0.62), 0.22),
			options
		)));
		scene.addHittable(std::make_shared<Mesh>(readObj(
			suzannePath,
			Vector3(0.35, 0.0, 3.2),
			Vector3(0.0, -25.0, 0.0),
			Vector3(0.7, 0.7, 0.7),
			std::make_shared<Lambertian>(Color(0.68, 0.38, 0.26)),
			options
		)));
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
	else if (benchmarkCase == "lights")
	{
		benchmarkLights(scene);
	}
	else if (benchmarkCase == "emissive-geometry")
	{
		benchmarkEmissiveGeometry(scene);
	}
	else if (benchmarkCase == "primitives-materials")
	{
		benchmarkPrimitivesMaterials(scene);
	}
	else if (benchmarkCase == "volumes")
	{
		benchmarkVolumes(scene);
	}
	else if (benchmarkCase == "obj-mesh")
	{
		benchmarkObjMesh(scene);
	}
	else
	{
		throw std::runtime_error("Unknown benchmark case: " + benchmarkCase);
	}
}
