#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile.hpp"
#include "ImageFiles/TIFF.hpp"
#include "OBJReader.hpp"
#include "Material.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Plane.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Mesh.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ImageFiles/ImageFileTypes.hpp"

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
		scene.setXResolution(256);
		scene.setYResolution(256);
		scene.setSampleCount(10);
		scene.setMaxLightBounces(24);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		//scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 64, 24, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(1.0, 1.0, 1.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		//scene.addCamera(Camera(Vector3(5.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0), 65, 0.0, 1.0));
		scene.addCamera(Camera(Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 1.0));

		// Mesh mesh = readObj("objects/blender_cube.obj");
		// scene.addHittable(std::make_shared<Mesh>(mesh));

		scene.addHittable(std::make_shared<Plane>(
			-1.0,
			Vector3(0.0, 1.0, 0.0),
			Material(Color(0.6, 0.6, 0.6), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		));

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

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(1, 1, 1),
			2,
			Material(Color(0.6, 0.6, 1.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
		));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.saveRenderToFile("render", TIFF_FILE);
	}

	return (0);
}
