#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile/SceneFile.hpp"
#include "OBJReader.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Mesh.hpp"
#include "Forms/Rectangle.hpp"
#include "Forms/Cube.hpp"
#include "Forms/ConstantVolume.hpp"
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

	srand(time(0));

	Scene scene;
	if (argc == 2)
	{
		SceneFile::read(scene, argv[1]);
	}
	else
	{
		scene.setYResolution(512);
		scene.setXResolution(512);
		scene.setSampleCount(100);
		scene.setMaxLightBounces(32);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_ATMOSPHERE);
		scene.setDistanceBlueness(false);
		scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 48, 24, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		//scene.setBackgroundColor(Color(0.1, 0.1, 0.1)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		scene.addCamera(Camera(Vector3(0.0, D_EARTH_RADIUS + 5.0, 0.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 1.0));

		scene.addHittable(std::make_shared<Plane>(
			D_EARTH_RADIUS,
			Vector3(0.0, 1.0, 0.0),
			std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))
		));

		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, D_EARTH_RADIUS + 1.0, -15.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0)),
			5.0,
			5.0,
			std::make_shared<Metal>(Color(1.0, 1.0, 1.0), 0.0)
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(-3.0, D_EARTH_RADIUS + 5.0, -8.0),
			1.0,
			std::make_shared<Dielectric>(Color(1.0, 1.0, 1.0))
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(3.0, D_EARTH_RADIUS + 5.0, -8.0),
			1.0,
			std::make_shared<Lambertian>(Color(0.2, 0.8, 0.2))
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(0.0, D_EARTH_RADIUS + 1.0, -17.0),
			1.0,
			std::make_shared<Emissive>(Color(1.0, 1.0, 1.0), 10.0)
		));

		scene.addHittable(std::make_shared<Mesh>(readObj(
				"objects/blender_lamp.obj",
				Vector3(0.0, D_EARTH_RADIUS + 3.0, -20.0),
				std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0)
			))
		));

		scene.addHittable(std::make_shared<ConstantVolume>(
			std::make_shared<Sphere>(
				Vector3(0.0, D_EARTH_RADIUS + 10.0, -20.0),
				3.4,
				nullptr
			),
			std::make_shared<Isotropic>(Color(1.0, 1.0, 1.0)),
			0.42
		));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.saveRenderToFile("cpu", BMP_FILE);
	}

	return (0);
}
