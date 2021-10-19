#include "Renderer.hpp"

static Color	calculatePixelColor(Scene scene, int x, int y);
static bool	checkHits(Scene scene, Ray ray, Color& pixelColor);

void	render(Scene scene)
{
	for (int y = 0; y < scene.getYResolution(); y++)
	{
		for (int x = 0; x < scene.getXResolution(); x++)
		{
			Color pixelColor(0.0f, 0.0f, 0.0f, 0.0f);

			for (int samples = 0; samples < D_SAMPLE_COUNT; samples++)
			{
				pixelColor += calculatePixelColor(scene, x, y);
			}
			pixelColor /= float(D_SAMPLE_COUNT);
			pixelColor = Color(sqrtf(pixelColor.getRed()), sqrtf(pixelColor.getGreen()), sqrtf(pixelColor.getBlue()), 0.0f); // Gamma (2) correction

			scene.setPixelArray((y * scene.getXResolution()) + x, pixelColor);
		}
	}
}

static Color	calculatePixelColor(Scene scene, int x, int y)
{
	Color	pixelColor(0.0f, 0.0f, 0.0f, 0.0f);
	Color	tempColor(0.0f, 0.0f, 0.0f, 0.0f);

	float u = float(x + drand48()) / (float)scene.getXResolution();
	float v = float(y + drand48()) / (float)scene.getYResolution();

    float   halfHeight = tan(((float)scene.getActiveCamera().getFOV() * M_PI / 180.0f) / 2.0f);
    float   halfWidth = ((float)scene.getXResolution() / (float)scene.getYResolution()) * halfHeight;
	Vector3	lowerLeftCorner = Vector3(-halfWidth, -halfHeight, -1.0f);
	Vector3	horizontal = Vector3(2.0f * halfWidth, 0.0f, 0.0f);
	Vector3	vertical = Vector3(0.0f, 2.0f * halfHeight, 0.0f);

	Ray ray(scene.getActiveCamera().getTransform().getPosition(), lowerLeftCorner + (horizontal * u) + (vertical * v) - scene.getActiveCamera().getTransform().getPosition());
	int	bounces;
	for (bounces = -1; bounces < D_MAX_LIGHT_BOUNCES && checkHits(scene, ray, tempColor); bounces++)
	{
		Vector3	newTarget = ray.hitRecord.position + ray.hitRecord.normal + randomPointInsideUnitSphere();
		ray.setOrigin(ray.hitRecord.position);
		ray.setDirection(newTarget - ray.hitRecord.position);
		pixelColor += tempColor / 2.0f;
		tempColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
	}
	if (bounces > 0)
	{
		pixelColor /= float(bounces);
	}
	return (pixelColor);
}

static bool	checkHits(Scene scene, Ray ray, Color& pixelColor)
{
	bool	anyHit = false;
	float	currentClosestObject = T_MAX;

	for (Sphere sphere : scene.getSpheres())
	{
		if (hitSphere(ray, sphere, currentClosestObject))
		{
			pixelColor = sphere.getMaterial().getColor();
			currentClosestObject = ray.hitRecord.t;
			anyHit = true;
		}
	}
	return (anyHit);
}
