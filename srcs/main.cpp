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

	FlagsParser(argc, argv).parse(scene);
	if (!scene.getIsFromFile())
	{
		scene.getImage()->setWidth(500);
		scene.getImage()->setHeight(500);
		scene.getImage()->initialize();
		scene.setSampleCount(2000);
		scene.setMaxLightBounces(12);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		// scene.setAtmosphere(Atmosphere(0.28, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(0.0, 0.0, 0.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		// A bigger aperture means more focus. ?(real aperture == aperture parameter / focus distance)?
		scene.addCamera(Camera(Vector3(-4.5, 1.0, 4.5), Vector3(0.5, 0.0, -0.5), 39.31, 0.0, 20.0)); // 35 mm

		scene.addHittable(std::make_shared<Plane>(
			0.0,
			Vector3(0.0, 1.0, 0.0),
			std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
		));

		scene.addHittable(std::make_shared<Sphere>(
			Vector3(0.0, 1.0, 0.0),
			1.0,
			std::make_shared<Dielectric>(Color(1.0, 1.0, 2.0))//Color(0.42, 0.42, 1.0))
		));

		scene.addHittable(std::make_shared<Rectangle>(
			Transform(Vector3(-15.0, 2.5, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0)),
			10.0,
			5.0,
			std::make_shared<Emissive>(Color(1.0, 1.0, 1.0), 10.0)
		));

		// scene.addHittable(std::make_shared<Rectangle>(
		// 	Transform(Vector3(-10.0, 2.5, -5.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0)),
		// 	10.0,
		// 	5.0,
		// 	std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
		// ));

		// scene.addHittable(std::make_shared<Rectangle>(
		// 	Transform(Vector3(-10.0, 2.5, 5.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 0.0, 0.0)),
		// 	10.0,
		// 	5.0,
		// 	std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
		// ));

		// scene.addHittable(std::make_shared<Rectangle>(
		// 	Transform(Vector3(-10.0, 5.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 0.0, 0.0)),
		// 	10.0,
		// 	10.0,
		// 	std::make_shared<Lambertian>(Color(1.0, 1.0, 1.0))
		// ));
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
