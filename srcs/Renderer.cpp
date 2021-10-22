#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Color.hpp"
#include "Vector2.hpp"
#include "Ray.hpp"
#include "HitUtils.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include <cmath>
#include <iostream>
#include <stdlib.h>

// Static function prototypes
static Color	calculatePixelColor(Scene scene, int x, int y);
static bool		checkHits(Scene scene, Ray& ray);
Color			calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
void			calculateLightRayBounceDirection(Ray& ray);

// Renders the image using all the information present on 'scene'. (Objects, cameras, lights, settings, etc)
void	render(Scene scene)
{
	std::cout << CLR_YELLOW << "Rendering..." << CLR_YELLOW << " (" << CLR_WHITE << scene.getSampleCount() << CLR_CYAN << " sample"
		<< pluralOrSingular(scene.getSampleCount()) << ", " << CLR_WHITE << scene.getMaxLightBounces() << CLR_CYAN << " max light bounce"
		<< pluralOrSingular(scene.getMaxLightBounces()) << CLR_YELLOW << ")\n" << CLR_RESET;

	Clock	clock;
	int		height = scene.getYResolution();
	int		width = scene.getXResolution();
	int		sampleCount = scene.getSampleCount();
	int		percentageUpdateFactor = height / 100;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Color pixelColor(0.0f, 0.0f, 0.0f, 0.0f);

			int samples;
			for (samples = 0; samples < sampleCount; samples++)
			{
				pixelColor += calculatePixelColor(scene, x, y);
			}
			pixelColor /= float(samples + 1);
			if (scene.getGammaCorrected() == true)
			{
				pixelColor = Color(sqrtf(pixelColor.getRed()), sqrtf(pixelColor.getGreen()), sqrtf(pixelColor.getBlue()), 0.0f); // Gamma (2) correction
			}

			scene.setPixelArray((y * width) + x, pixelColor);
		}
		if (y % percentageUpdateFactor == 0)
		{
			int percentage = (float(y) / float(height)) * 100.0f;
			std::cout << CLR_WHITE << "\r[ " << percentage << "% ]" << std::flush;
		}
	}

	double elapsedS = clock.stop();
	std::cout << CLR_WHITE << "\r[ 100% ]";
	std::cout << CLR_GREEN_BRIGHT << "\nRender done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << elapsedS << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
}

// Calculates the color for the pixel at 'x' and 'y'. Creates rays, checks for intersections with objects on 'scene' and bounce light rays
static Color	calculatePixelColor(Scene scene, int x, int y)
{
	Color	tempColor(0.0f, 0.0f, 0.0f, 0.0f);

	float u = float(x + drand48()) / (float)scene.getXResolution();
	float v = float(y + drand48()) / (float)scene.getYResolution();

    static float   halfHeight = tan(((float)scene.getActiveCamera().getFOV() * M_PI / 180.0f) / 2.0f);
    static float   halfWidth = ((float)scene.getXResolution() / (float)scene.getYResolution()) * halfHeight;

	static Vector3	lowerLeftCorner = Vector3(-halfWidth, -halfHeight, -1.0f);
	static Vector3	horizontal = Vector3(2.0f * halfWidth, 0.0f, 0.0f);
	static Vector3	vertical = Vector3(0.0f, 2.0f * halfHeight, 0.0f);

	Ray ray(scene.getActiveCamera().getTransform().getPosition(), lowerLeftCorner + (horizontal * u) + (vertical * v) - scene.getActiveCamera().getTransform().getPosition());
	return (calculateLightRaysColor(ray, scene, 0));
}

Color	calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	if (bounces < scene.getMaxLightBounces() && checkHits(scene, ray))
	{
		ray.setOrigin(ray.hitRecord.position);

		calculateLightRayBounceDirection(ray);

		Color color = (calculateLightRaysColor(ray, scene, bounces + 1) + ray.hitRecord.material.getColor()) / 2.0f;

		return (color * ray.hitRecord.material.getAlbedo());
	}
	return (Color(0.0f, 0.0f, 0.0f, 0.0f));
}

void	calculateLightRayBounceDirection(Ray& ray)
{
	if (ray.hitRecord.material.getMetallic() == 1.0f)
	{
		ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal) + (randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
		return;
	}

	Vector3	newTarget = ray.hitRecord.position + ray.hitRecord.normal + randomPointInsideUnitSphere();
	if (ray.hitRecord.material.getMetallic() == 0.0f)
	{

		ray.setDirection(newTarget - ray.hitRecord.position);
		return;
	}

	if (drand48() < ray.hitRecord.material.getMetallic())
	{
		ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal) + (randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
	}
	else
	{
		ray.setDirection(newTarget - ray.hitRecord.position);
	}
}

// Checks if 'ray' hits objects present 'scene'. On hit, sets 'pixelColor' to the hitted object's material color
static bool	checkHits(Scene scene, Ray& ray)
{
	bool	anyHit = false;
	float	currentClosestObject = T_MAX;

	for (Sphere sphere : scene.getSpheres())
	{
		if (hitSphere(ray, sphere, currentClosestObject))
		{
			ray.hitRecord.material = sphere.getMaterial();
			currentClosestObject = ray.hitRecord.t;
			anyHit = true;
		}
	}
	return (anyHit);
}
