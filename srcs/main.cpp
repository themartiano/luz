#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile.hpp"
#include "OBJReader.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Mesh.hpp"
#include "Forms/Rectangle.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ImageFiles/Types.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Dielectric.hpp"

// Main function
int	main(int argc, char *argv[])
{
	std::cout << CLR_BLUE << "Preparing..." << CLR_RESET << std::endl << std::endl;

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
		scene.setMaxLightBounces(24);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		//scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 64, 24, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(0.0, 0.0, 0.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		//scene.addCamera(Camera(Vector3(0.0, 5.0, 0.0), Vector3(0.0, -1.0, -T_MIN), 65, 0.0, 1.0));
		scene.addCamera(Camera(Vector3(0.0, 1.0, 5.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 1.0));

		scene.addHittable(std::make_shared<Plane>(
			0.0,
			Vector3(0.0, 1.0, 0.0),
			std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6))
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(0.0, 1.0, 0.0),
			1.0,
			std::make_shared<Dielectric>(Color(1.0, 1.0, 1.0))
		));

		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(0.0, 1.0, -8.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
			10.0,
			10.0,
			std::make_shared<Emissive>(Color(1.0, 1.0, 1.0), 20)
		));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.saveRenderToFile(BMP_FILE);
	}

	return (0);
}
