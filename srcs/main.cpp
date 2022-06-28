#include "Scene/Scene.hpp"
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
#include "Image.hpp"
#include "Charts/Bar.hpp"
#include "FlagsParser.hpp"
#include "Scene/SceneHelpers.hpp"
#include <memory>

// Main function
int	main(int argc, char *argv[])
{
	Scene scene;
	scene.setStorePixelRenderTimes(true); // Test so it works for scene files (before it's natively there)

	std::cout << CLR_BLUE << "Preparing..." << CLR_RESET << std::endl << std::endl;

	FlagsParser(argc, argv).parse(scene);
	if (!scene.getIsFromFile())
	{
		scene.getImage()->setWidth(1000);
		scene.getImage()->setHeight(1000);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setMaxLightBounces(50);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		// scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(0.0, 0.0, 0.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		SceneHelpers::cornellBox(scene);

		// scene.addHittable(std::make_shared<Mesh>(readObj("objects/blender_monkey.obj", Vector3(0.0, -25.0, -100.0), std::make_shared<Dielectric>(Color(0.42, 0.42, 0.42)))));
	}

	if (Renderer::render(scene))
	{
		// Writes render image file
		scene.getImage()->saveToBMP("render");
		std::unique_ptr<Image> debugTime = scene.generateRenderTimeImage();
		debugTime->saveToBMP("renderTime");
	}

	return (0);
}
