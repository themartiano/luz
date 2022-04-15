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
#include "Hittables/Landscape.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ImageFiles/Types.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Isotropic.hpp"
#include "Noise/Perlin.hpp"

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
		scene.setYResolution(500);
		scene.setXResolution(500);
		scene.setSampleCount(3);
		scene.setMaxLightBounces(6);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		// scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(1.0, 1.0, 1.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		scene.addCamera(Camera(Vector3(0.0, 10.0, 0.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 20.0));

		scene.addHittable(std::make_shared<Landscape>(
			Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			std::make_shared<Lambertian>(Color(0.3, 0.29, 0.11)),
			42
		));

		// scene.addHittable(std::make_shared<Plane>(
		// 	D_EARTH_RADIUS,
		// 	Vector3(0.0, 1.0, 0.0),
		// 	std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))
		// ));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.saveRenderToFile("render", BMP_FILE);
	}

	return (0);
}
