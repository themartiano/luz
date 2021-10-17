#include "Renderer.hpp"

void	render(Scene scene)
{
	for (int y = 0; y < scene.getYResolution(); y++)
	{
		for (int x = 0; x < scene.getXResolution(); x++)
		{
			scene.setPixelArray((y * scene.getXResolution()) + x, 122, 122, 122);
		}
	}
}
