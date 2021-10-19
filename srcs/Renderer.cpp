#include "Renderer.hpp"

static Color	calculatePixelColor(Scene scene, int x, int y);

void	render(Scene scene)
{
	scene.addSphere(Sphere(Transform(Vector3(0.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)), Material(Color(126, 126, 126, 0), 1.0f), 1.0f));
	for (int y = 0; y < scene.getYResolution(); y++)
	{
		for (int x = 0; x < scene.getXResolution(); x++)
		{
			Color color = calculatePixelColor(scene, x, y);
			scene.setPixelArray((y * scene.getXResolution()) + x, color.getRed(), color.getGreen(), color.getBlue());
		}
	}
}

static Color	calculatePixelColor(Scene scene, int x, int y)
{
	Color	pixelColor(0, 0, 0, 0);

	Vector2 pixel((float)x + drand48() / (float)scene.getXResolution(), (float)y + drand48() / (float)scene.getYResolution());
	float u = float(x) / float(scene.getXResolution());
	float v = float(y) / float(scene.getYResolution());
	Ray ray(scene.getActiveCamera().getTransform().getPosition(), Vector3(-2.0f, -1.0f, -1.0f) + (Vector3(4.0f, 0.0f, 0.0f) * u) + (Vector3(0.0f, 2.0f, 0.0f) * v));
	for (Sphere sphere : scene.getSpheres())
	{
		if (sphere.hit(ray))
		{
			return (sphere.getMaterial().getColor());
		}
	}
	return (pixelColor);
}
