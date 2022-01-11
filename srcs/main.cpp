#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile.hpp"
#include "BMP.hpp"
#include "OBJReader.hpp"
#include "Material.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Sphere.hpp"
#include "Utilities.hpp"

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
		scene.setMaxLightBounces(8);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		//scene.setAtmosphere(Atmosphere(-0.189, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(1.0, 1.0, 1.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		//scene.addCamera(Camera(Vector3(5.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0), 65, 0.0, 1.0));
		scene.addCamera(Camera(Vector3(0.0, 2.0, 12.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 1.0));

		readObj(scene, "objects/lamp.obj");

		// scene.addHittable(std::make_shared<Triangle>(
		// 	Vector3(1, 0, 0),
		// 	Vector3(0, 1, 0),
		// 	Vector3(0, 0, 0),
		// 	Material(Color(0.6, 1.0, 0.6), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));

		// scene.addHittable(std::make_shared<Triangle>(
		// 	Vector3(0, 0, 0),
		// 	Vector3(0, 1, 0),
		// 	Vector3(1, 0, 0),
		// 	Material(Color(0.6, 0.6, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));

		// scene.addHittable(std::make_shared<Sphere>(
		// 	Vector3(1, 1, 1),
		// 	2,
		// 	Material(Color(0.6, 0.6, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));

		// scene.addHittable(std::make_shared<Plane>(
		// 	-1.0,
		// 	Vector3(0.0, 1.0, 0.0),
		// 	Material(Color(0.6, 0.6, 0.6), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		// ));
	}

	if (Renderer::render(scene))
	{
		// Writes BMP image file
		BMP::writeFile(scene);
	}

	return (0);
}
