#include "Scene.hpp"
#include "BMP.hpp"
#include "Renderer/Renderer.hpp"
#include "SkyTypes.hpp"
#include "Defaults.hpp"
#include "Atmosphere.hpp"

// Renders a sequence of frames
void	renderSequence(Scene& scene, Atmosphere baseAtmosphere, int fps, double duration)
{
	scene.setRenderSky(SKY_ATMOSPHERE);

	int totalFPS = fps * duration;
	double sunChangePerFrame = 0.6 / totalFPS;

	double sunPosition = baseAtmosphere.getSunAngle();
	for (int i = 0; i < totalFPS; i++)
	{
		baseAtmosphere.setSunAngle(sunPosition);
		scene.setAtmosphere(baseAtmosphere);
		sunPosition -= sunChangePerFrame;

		Renderer::render(scene);
		scene.setOutputFileName("sequenceFrame" + std::to_string(i));
		BMP::writeFile(scene, true, "sequence");
	}
}
