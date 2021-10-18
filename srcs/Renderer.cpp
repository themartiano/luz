#include "Renderer.hpp"

static Color	calculatePixelColor(Scene scene, int x, int y);

void	render(Scene scene)
{
	for (int y = 0; y < scene.getYResolution(); y++)
	{
		for (int x = 0; x < scene.getXResolution(); x++)
		{
			Color color = calculatePixelColor(scene, x, y);
			scene.setPixelArray((y * scene.getXResolution()) + x, color.getRed(), color.getGreen(), color.getBlue());
		}
	}
}
#include <iostream>
static Color	calculatePixelColor(Scene scene, int x, int y)
{
	Color	pixelColor(0, 0, 0, 0);

	Vector2 pixel((float)x + drand48() / (float)scene.getXResolution(), (float)y + drand48() / (float)scene.getYResolution());

	return (pixelColor);
}
