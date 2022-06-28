#include "Renderer/Renderer.hpp"
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

	std::cout << CLR_YELLOW << "Rendering with:" << CLR_RESET << std::endl;
	std::cout << CLR_GREEN << CORE_COUNT * THREAD_MULTIPLIER << CLR_BLUE << " threads;" << CLR_RESET << std::endl;
	std::cout << CLR_GREEN << scene.getSampleCount() << CLR_BLUE << " samples per pixel;" << CLR_RESET << std::endl;
	std::cout << CLR_GREEN << scene.getImage()->getHeight() << CLR_BLUE << " x " << CLR_GREEN << scene.getImage()->getWidth() << CLR_RESET << std::endl;

	Clock	clock;
	clock.start();

	internal::_manageThreads(scene);

	std::cout << "\r" << CLR_CYAN << "Progress: " << CLR_WHITE << "[ 100% ]";
	std::cout << CLR_GREEN_BRIGHT << "\nRender done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << clock.elapsedS() << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;

	return (true);
}
