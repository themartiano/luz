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

	float u = (float)x / (float)scene.getXResolution();
	float v = (float)y / (float)scene.getYResolution();

    float   halfHeight = tan(((float)scene.getActiveCamera().getFOV() * M_PI / 180.0f) / 2.0f);
    float   halfWidth = ((float)scene.getXResolution() / (float)scene.getYResolution()) * halfHeight;
	Vector3	lowerLeftCorner = Vector3(-halfWidth, -halfHeight, -1.0f);
	Vector3	horizontal = Vector3(2.0f * halfWidth, 0.0f, 0.0f);
	Vector3	vertical = Vector3(0.0f, 2.0f * halfHeight, 0.0f);

	Ray ray(scene.getActiveCamera().getTransform().getPosition(), lowerLeftCorner + (horizontal * u) + (vertical * v) - scene.getActiveCamera().getTransform().getPosition());
	float	currentClosestObject = T_MAX;
	for (Sphere sphere : scene.getSpheres())
	{
		if (hitSphere(ray, sphere, currentClosestObject))
		{
			pixelColor = sphere.getMaterial().getColor();
		}
	}
	return (pixelColor);
}
