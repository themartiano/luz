#include "Renderer/Renderer.hpp"
#include "RendererInternal.hpp"
#include "ANSIColors.hpp"
#include "Clock.hpp"
#include "Defaults.hpp"
#include "Renderer/CausticPhotonMap.hpp"
#include "Random.hpp"

// Renders the image using all the information present on 'scene'. (Objects, cameras, lights, settings, etc)
bool	Renderer::render(Scene& scene)
{
	if (!scene.hasCamera())
	{
		std::cerr << CLR_RED << "No camera on the Scene." << CLR_RESET << std::endl;
		return (false);
	}

	scene.syncAtmosphereSunDirection();
	scene.updateLights();
	scene.updateAccelerationStructure();

	if (!scene.getBenchmarkMode())
	{
		std::cout << CLR_YELLOW << "Rendering with:" << CLR_RESET << std::endl;
		std::cout << CLR_GREEN << scene.getRenderingThreads() << CLR_BLUE << " threads;" << CLR_RESET << std::endl;
		std::cout << CLR_GREEN << scene.getSampleCount() << CLR_BLUE << " max samples per pixel;" << CLR_RESET << std::endl;
		if (scene.getAdaptiveSampling())
		{
			std::cout
				<< CLR_GREEN << "adaptive" << CLR_BLUE << " sampling; "
				<< CLR_GREEN << scene.getAdaptiveMinSamples() << CLR_BLUE << " min spp; "
				<< CLR_GREEN << scene.getAdaptiveThreshold() << CLR_BLUE << " threshold;"
				<< CLR_RESET << std::endl;
		}
		std::cout << CLR_GREEN << scene.getImage()->getWidth() << CLR_BLUE << " x " << CLR_GREEN << scene.getImage()->getHeight() << CLR_RESET << std::endl;
	}

	Clock	clock;
	clock.start();
	if (scene.getCausticsEnabled() && scene.getCausticPhotonCount() > 0)
	{
		const std::uint32_t renderSeed = hasRandomSeed()
			? static_cast<std::uint32_t>(randomSeedValue())
			: randomEngine.integer();
		auto causticPhotonMap = std::make_shared<CausticPhotonMap>();
		causticPhotonMap->build(scene, renderSeed);
		scene.setCausticPhotonMap(causticPhotonMap);
	}
	else
	{
		scene.setCausticPhotonMap(nullptr);
	}

	internal::_manageThreads(scene);

	const double totalMS = clock.elapsedMS();
	SceneRenderStats stats = scene.getRenderStats();
	stats.totalMS = totalMS;
	scene.setRenderStats(stats);

	if (!scene.getBenchmarkMode())
	{
		std::cout << CLR_GREEN_BRIGHT << "Render done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << totalMS / 1000.0 << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
	} else {
		std::cout
			<< "stats"
			<< " rendered_samples=" << stats.renderedSamples
			<< " avg_spp=" << stats.averageSamplesPerPixel
			<< " render_ms=" << stats.renderMS
			<< " denoise_ms=" << stats.denoiseMS
			<< " postprocess_ms=" << stats.postProcessMS
			<< " total_ms=" << stats.totalMS
			<< std::endl;
		std::cout << totalMS << std::endl;
	}

	return (true);
}
