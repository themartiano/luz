#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "ANSIColors.hpp"
#include "SceneFile.hpp"
#include "BMP.hpp"

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
		scene.setMaxLightBounces(4);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		//scene.setAtmosphere(Atmosphere(-0.189, D_EARTH_RADIUS, D_ATMOSPHERE_RADIUS, D_HR, D_HM, 16, 8, 0.468)); // Only needed if Scene.Sky == SKY_ATMOSPHERE
		scene.setBackgroundColor(Color(1.0, 1.0, 1.0)); // Only needed if Scene.Sky == SKY_NONE

		// Coordinate system ~~ Right Hand ~~ Forward: -Z | Up: +Y | Right: +X

		scene.addCamera(Camera(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, -1.0), 65, 0.0, 1.0));
	}

	if (Renderer::render(scene))
	{
		// Writes BMP image file
		BMP::writeFile(scene);
	}

	return (0);
}
