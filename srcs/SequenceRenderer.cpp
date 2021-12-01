#include "Scene.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "SkyTypes.hpp"
#include "Defaults.hpp"
#include "Atmosphere.hpp"

// Renders a sequence of frames.
void    renderSequence(Scene& scene, Atmosphere baseAtmosphere, int fps, double duration)
{
	scene.setRenderSky(SKY_ATMOSPHERE);

    int totalFPS = fps * duration;
    double sunChangePerFrame = 0.6 / totalFPS;

    BMP frame;
    double sunPosition = baseAtmosphere.getSunAngle();
    for (int i = 0; i < totalFPS; i++)
    {
        baseAtmosphere.setSunAngle(sunPosition);
        scene.setAtmosphere(baseAtmosphere);
        sunPosition -= sunChangePerFrame;

        render(scene);
        frame = BMP("sequenceFrame" + std::to_string(i));
        frame.writeFile(scene, true, "sequence");
    }
}
