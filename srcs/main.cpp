#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile/SceneFile.hpp"
#include "OBJReader.hpp"
#include "Hittables/Triangle.hpp"
#include "Hittables/Plane.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Mesh.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/ConstantVolume.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ImageFiles/Types.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Isotropic.hpp"

// Main function
int	main(int argc, char *argv[])
{
	std::cout << CLR_BLUE << "Preparing..." << CLR_RESET << std::endl << std::endl;

	//srand(time(0));

	Scene scene;
	if (argc == 2)
	{
		SceneFile::read(scene, argv[1]);
	}
	else
	{
		scene.setYResolution(500);
		scene.setXResolution(500);
		scene.setSampleCount(1000);
		scene.setMaxLightBounces(32);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_ATMOSPHERE);
		scene.setDistanceBlueness(false);
		scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 64, 32, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		// scene.setBackgroundColor(Color(1.0, 1.0, 1.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		srand(845);
		scene.addCamera(Camera(Vector3(0.0, D_EARTH_RADIUS + 5.0, 0.0), Vector3(0.0, 0.0, -1.0), 65, 0.5, 20.0));

		scene.addHittable(std::make_shared<Plane>(
			D_EARTH_RADIUS,
			Vector3(0.0, 1.0, 0.0),
			std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))
		));

		// scene.addHittable(std::make_shared<Rectangle>(
		// 	Transform(Vector3(0.0, D_EARTH_RADIUS, -15.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
		// 	5.0,
		// 	5.0,
		// 	std::make_shared<Metal>(Color(1.0, 1.0, 1.0), 0.0)
		// ));

		scene.addHittable(std::make_shared<Mesh>(readObj(
				"objects/blender_glass.obj",
				Vector3(0.0, D_EARTH_RADIUS + 5.0, -20.0),
				std::make_shared<Dielectric>(Color(0.76, 0.76, 0.76))
			)
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(7.0, D_EARTH_RADIUS + 5.0, -20.0),
			2.5,
			std::make_shared<Metal>(Color(1.0, 1.0, 1.0), 0.0)
		));

			scene.addHittable(std::make_shared<Sphere>(
			Vector3(-7.0, D_EARTH_RADIUS + 5.0, -20.0),
			2.5,
			std::make_shared<Metal>(Color(1.0, 1.0, 1.0), 0.68)
		));

		std::vector<std::shared_ptr<Hittable>> floorSpheres;
		for (int x = 100; x > -100; x--)
		{
			for (int z = 500; z > -500; z--)
			{
				if (rand() % 12 == 0)
				{
					Vector3 position = Vector3(x, D_EARTH_RADIUS + 0.5, z);

					int random = rand() % 10;

					if (random <= 3) // Lambertian
					{
						floorSpheres.push_back(std::make_shared<Sphere>(
							position,
							0.5,
							std::make_shared<Lambertian>(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble()))
						));
					}
					else if (random == 4 || random == 5) // Metal
					{

						floorSpheres.push_back(std::make_shared<Sphere>(
							position,
							0.5,
							std::make_shared<Metal>(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble()), Utilities::randomDouble())
						));
					}
					else if (random == 6 || random == 7) // Dielectric
					{

						floorSpheres.push_back(std::make_shared<Sphere>(
							position,
							0.5,
							std::make_shared<Dielectric>(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble()))
						));
					}
					else if (random == 8) // Volume
					{
						floorSpheres.push_back(std::make_shared<ConstantVolume>(
							std::make_shared<Sphere>(
								position,
								0.5,
								nullptr
							),
							std::make_shared<Isotropic>(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble())),
							Utilities::randomDouble(0.1, 10.0)
						));
					}
					else if (random == 9) // Emissive
					{

						floorSpheres.push_back(std::make_shared<Sphere>(
							position,
							0.5,
							std::make_shared<Emissive>(Color(Utilities::randomDouble(), Utilities::randomDouble(), Utilities::randomDouble()), Utilities::randomDouble(5.0, 12.6))
						));
					}
				}
			}
		}

		scene.addHittable(std::make_shared<BVHNode>(floorSpheres));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.saveRenderToFile("render", BMP_FILE);
	}

	return (0);
}
