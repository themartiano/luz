#include "Renderer/Renderer.hpp"
#include "RendererInternal.hpp"
#include "ANSIColors.hpp"
#include "Clock.hpp"
#include "Defaults.hpp"

// Renders the image using all the information present on 'scene'. (Objects, cameras, lights, settings, etc)
bool	Renderer::render(Scene& scene)
{
	if (!scene.hasCamera())
	{
		std::cerr << CLR_RED << "No camera on the Scene." << CLR_RESET << std::endl;
		return (false);
	}

	scene.updateLights();
	scene.updateAccelerationStructure();

	if (!scene.getBenchmarkMode())
	{
		std::cout << CLR_YELLOW << "Rendering with:" << CLR_RESET << std::endl;
		std::cout << CLR_GREEN << scene.getRenderingThreads() << CLR_BLUE << " threads;" << CLR_RESET << std::endl;
		std::cout << CLR_GREEN << scene.getSampleCount() << CLR_BLUE << " samples per pixel;" << CLR_RESET << std::endl;
		std::cout << CLR_GREEN << scene.getImage()->getWidth() << CLR_BLUE << " x " << CLR_GREEN << scene.getImage()->getHeight() << CLR_RESET << std::endl;
	}

	Clock	clock;
	clock.start();

	internal::_manageThreads(scene);

	if (!scene.getBenchmarkMode())
	{
		std::cout << CLR_GREEN_BRIGHT << "Render done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << clock.elapsedS() << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
	} else {
		std::cout << clock.elapsedMS() << std::endl;
	}

	return (true);
}
