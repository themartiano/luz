#include "Scene.hpp"
#include "ImageFiles/BMP.hpp"
#include "Renderer/Renderer.hpp"
#include "SkyTypes.hpp"
#include "Defaults.hpp"
#include "Atmosphere.hpp"
#include "ImageFiles/ImageFileTypes.hpp"

// Renders a sequence of frames
void	Renderer::renderSequence(Scene& scene, Atmosphere baseAtmosphere, int fps, double duration, ImageFileTypes imageFileType)
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
		scene.saveRenderToFile("sequenceFrame" + std::to_string(i), imageFileType);
	}
}
