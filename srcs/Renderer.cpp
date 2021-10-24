#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Color.hpp"
#include "Vector2.hpp"
#include "Ray.hpp"
#include "HitUtils.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include "RefractiveIndexes.hpp"
#include <cmath>
#include <iostream>
#include <stdlib.h>

// Static function prototypes
static Color	calculatePixelColor(Scene scene, int x, int y);
static bool		checkHits(Scene scene, Ray& ray);
Color			calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
void			calculateLightRayBounceDirection(Ray& ray, Color& color);
Color			calculateSkyInterpolation(Scene scene, Ray ray);

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
			Color pixelColor(0.0f, 0.0f, 0.0f);

			int samples;
			for (samples = 0; samples < sampleCount; samples++)
			{
				pixelColor += calculatePixelColor(scene, x, y);
			}
			pixelColor /= float(sampleCount);
			if (scene.getGammaCorrected() == true)
			{
				pixelColor = Color(sqrtf(pixelColor.getRed()), sqrtf(pixelColor.getGreen()), sqrtf(pixelColor.getBlue())); // Gamma (2) correction
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
	float xU = float(x + drand48()) / (float)scene.getXResolution();
	float yV = float(y + drand48()) / (float)scene.getYResolution();

	static Vector3	cameraPosition = scene.getActiveCamera().getLookFrom();

    static float	halfWidth = tan((((float)scene.getActiveCamera().getFOV() * M_PI) / 180.0f) / 2.0f);
    static float	halfHeight = ((float)scene.getYResolution() / (float)scene.getXResolution()) * halfWidth;

	static float	lensRadius = scene.getActiveCamera().getAperture() / 2.0f;
	static float	focusDistance = vectorLength(cameraPosition - scene.getActiveCamera().getLookAt());

	static Vector3	w = Vector3(0, 0, 1);//normalize(cameraPosition - scene.getActiveCamera().getLookAt());
	static Vector3	viewUp(0.0f, -1.0f, 0.0f);
	static Vector3	u = normalize(cross(viewUp, w));
	static Vector3	v = cross(w, u);

	static Vector3	lowerLeftCorner = cameraPosition - (u * halfWidth * focusDistance) - (v * halfHeight * focusDistance) - (w * focusDistance);
	static Vector3	horizontal = u * (halfWidth * 2.0f * focusDistance);
	static Vector3	vertical = v * (halfHeight * 2.0f * focusDistance);

	Vector3	rd = randomPointInsideUnitDisk() * lensRadius;
	Vector3	offset = u * rd.getX() + v * rd.getY();

	Ray ray(cameraPosition + offset, lowerLeftCorner + (horizontal * xU) + (vertical * yV) - cameraPosition - offset);
	return (calculateLightRaysColor(ray, scene, 0));
}

// Properly calculates light rays bounces, reflections, etc and returns the resulting color
Color	calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	Color color(0.0f, 0.0f, 0.0f);

	if (bounces > scene.getMaxLightBounces())
	{
		return (color);
	}

	if (checkHits(scene, ray))
	{
		ray.setOrigin(ray.hitRecord.position);
		calculateLightRayBounceDirection(ray, color);

		if (ray.hitRecord.material.getMetallic() == 1.0f && dot(ray.getDirection(), ray.hitRecord.normal) <= 0.0f)
		{
			return (color);
		}

		return (color * calculateLightRaysColor(ray, scene, bounces + 1));
	}

	return (calculateSkyInterpolation(scene, ray));
}

// Calculates the sky interpolation for the background and reflexes
Color	calculateSkyInterpolation(Scene scene, Ray ray)
{
	Vector3	normalizedDirection = normalize(ray.getDirection());

	float temp = scene.getSkyline() * (-normalizedDirection.getY() + 1.0f);

	return ((Color(1.0f, 1.0f, 1.0f) * (1.0f - temp)) + (Color(0.5f, 0.7f, 1.0f) * temp));
}

// Calculates the light rays bounce/reflection direction
void	calculateLightRayBounceDirection(Ray& ray, Color& color)
{
	if (ray.hitRecord.material.getMetallic() == 1.0f)
	{
		ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal) + (randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
		return;
	}

	if (ray.hitRecord.material.getIsDielectric() == true)
	{
		Vector3	refractedVector;
		Vector3	outwardsNormal;
		float	reflectionProbability;
		float	cosine;
		float	refractiveIndex = RI_GLASS;
		float	directionNormalDot = dot(ray.getDirection(), ray.hitRecord.normal);

		if (directionNormalDot > 0.0f)
		{
			outwardsNormal = ray.hitRecord.normal * -1.0f;
			cosine = refractiveIndex * directionNormalDot / vectorLength(ray.getDirection());
		}
		else
		{
			outwardsNormal = ray.hitRecord.normal;
			refractiveIndex = 1.0f / refractiveIndex;
			cosine = -directionNormalDot / vectorLength(ray.getDirection());
		}

		if (refract(ray.getDirection(), outwardsNormal, refractiveIndex, refractedVector))
		{
			reflectionProbability = schlick(cosine, refractiveIndex);
		}
		else
		{
			reflectionProbability = 1.0f;
		}

		if (drand48() < reflectionProbability)
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal));
		}
		else
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(refractedVector);
		}

		color = Color(1.0f, 1.0f, 1.0f);
		return;
	}

	Vector3	newTarget = ray.hitRecord.position + ray.hitRecord.normal + randomPointInsideUnitSphere();
	if (ray.hitRecord.material.getMetallic() == 0.0f)
	{
		ray.setDirection(newTarget - ray.hitRecord.position);
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
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
