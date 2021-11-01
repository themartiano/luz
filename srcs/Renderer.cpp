#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Color.hpp"
#include "Vector2.hpp"
#include "Ray.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include "RefractiveIndexes.hpp"
#include "SystemSpecifics.hpp"
#include "Atmosphere.hpp"
#include "Forms/Sphere.hpp"
#include <cmath>
#include <iostream>
#include <stdlib.h>

// Static function prototypes
static Color	calculatePixelColor(Scene& scene, int x, int y);
static bool		checkHits(Scene& scene, Ray& ray);
static Color	calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
static void		calculateLightRayBounceDirection(Ray& ray, Color& color);
static Color	calculateSkyInterpolation(Scene& scene, Ray& ray);

// Renders the image using all the information present on 'scene'. (Objects, cameras, lights, settings, etc)
void	render(Scene& scene)
{
	std::cout << CLR_YELLOW << "Rendering..." << CLR_YELLOW << " (" << CLR_WHITE << scene.getSampleCount() << CLR_CYAN << " sample"
		<< pluralOrSingular(scene.getSampleCount()) << ", " << CLR_WHITE << scene.getMaxLightBounces() << CLR_CYAN << " max light bounce"
		<< pluralOrSingular(scene.getMaxLightBounces()) << CLR_YELLOW << ")\n" << CLR_RESET;

	Clock	clock;
	int		height = scene.getYResolution();
	int		width = scene.getXResolution();
	int		sampleCount = scene.getSampleCount();
	int		percentageUpdateFactor = height / 100;
	bool	gammaCorrected = scene.getGammaCorrected();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Color pixelColor(0.0, 0.0, 0.0);

			for (int samples = 0; samples < sampleCount; samples++)
			{
				pixelColor += calculatePixelColor(scene, x, y);
			}
			pixelColor /= double(sampleCount);
			if (gammaCorrected)
			{
				pixelColor = Color(sqrtf(pixelColor.getRed()), sqrtf(pixelColor.getGreen()), sqrtf(pixelColor.getBlue())); // Gamma (2) correction
			}

			// Replaces NaN with zeros (in case there's a problematic sample)
			if (pixelColor.getRed() != pixelColor.getRed())
			{
				pixelColor.setRed(0.0);
			}
			if (pixelColor.getGreen() != pixelColor.getGreen())
			{
				pixelColor.setGreen(0.0);
			}
			if (pixelColor.getBlue() != pixelColor.getBlue())
			{
				pixelColor.setBlue(0.0);
			}

			scene.setPixelArray((y * width) + x, pixelColor);
		}
		if (y % percentageUpdateFactor == 0)
		{
			int percentage = (double(y) / double(height)) * 100.0;
			std::cout << CLR_WHITE << "\r[ " << percentage << "% ]" << std::flush;
		}
	}

	double elapsedS = clock.stop();
	std::cout << CLR_WHITE << "\r[ 100% ]";
	std::cout << CLR_GREEN_BRIGHT << "\nRender done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << elapsedS << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
}

// Calculates the color for the pixel at 'x' and 'y'. Creates rays, checks for intersections with objects on 'scene' and bounce light rays
static Color	calculatePixelColor(Scene& scene, int x, int y)
{
	static double width = double(scene.getXResolution());
	static double height = double(scene.getYResolution());

	double xU = double(x + randomdouble()) / width;
	double yV = double(y + randomdouble()) / height;

	static Vector3	cameraPosition = scene.getActiveCamera().getLookFrom();

    static double	halfWidth = tan((((double)scene.getActiveCamera().getFOV() * M_PI) / 180.0) / 2.0);
    static double	halfHeight = (height / width) * halfWidth;

	static double	lensRadius = scene.getActiveCamera().getAperture() / 2.0;
	static double	focusDistance = vectorLength(cameraPosition - scene.getActiveCamera().getLookAt());

	static Vector3	w = normalize(cameraPosition - scene.getActiveCamera().getLookAt());
	static Vector3	viewUp(0.0, -1.0, 0.0);
	static Vector3	u = normalize(cross(viewUp, w));
	static Vector3	v = cross(w, u);

	static Vector3	lowerLeftCorner = cameraPosition - (u * halfWidth * focusDistance) - (v * halfHeight * focusDistance) - (w * focusDistance);
	static Vector3	horizontal = u * (halfWidth * 2.0 * focusDistance);
	static Vector3	vertical = v * (halfHeight * 2.0 * focusDistance);

	Vector3	rd = randomPointInsideUnitDisk() * lensRadius;
	Vector3	offset = u * rd.getX() + v * rd.getY();

	Ray ray(cameraPosition + offset, lowerLeftCorner + (horizontal * xU) + (vertical * yV) - cameraPosition - offset);

	return (calculateLightRaysColor(ray, scene, 0));
}

// Properly calculates light rays bounces, reflections, etc and returns the resulting color
static Color	calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	static int maxLightBounces = scene.getMaxLightBounces();
	static Atmosphere	atmosphere;

	if (bounces > maxLightBounces)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	Color color, emitted = Color(0.0, 0.0, 0.0);
	if (checkHits(scene, ray))
	{
		ray.setOrigin(ray.hitRecord.position);

		if (ray.hitRecord.material.getIsEmissive() == true)
		{
			emitted = ray.hitRecord.material.getColor() * ray.hitRecord.material.getLightIntensity();
			return (emitted);
		}
		else
		{
			calculateLightRayBounceDirection(ray, color);
		}

		if ((ray.hitRecord.material.getMetallic() == 1.0 && dot(ray.getDirection(), ray.hitRecord.normal) <= 0.0))
		{
			return (emitted + color);
		}

		return (emitted + color * calculateLightRaysColor(ray, scene, bounces + 1));
	}

	static bool renderSky = scene.getRenderSky();
	if (renderSky == true)
	{
		Ray atmosphereRay(ray.getOrigin() + Vector3(0.0, atmosphere.getEarthRadius(), 0.0), ray.getDirection() * Vector3(0.0, -1.0, 0.0));

		double t_max = T_MAX;
		// Sphere  earth(Vector3(0.0, 0.0, 0.0), Material(), atmosphere.getEarthRadius());

		// // Checks ray collision with Earth
		// if (hitAtmosphere(earth, atmosphereRay) && atmosphereRay.hitRecord.t1 > 0.0)
		// {
		// 	t_max = std::max(0.0, atmosphereRay.hitRecord.t0);
		// }
		return (atmosphere.computeIncidentLight(atmosphereRay, t_max));
	}
	else
	{
		return (scene.getBackgroundColor());
	}
}

// Calculates the sky interpolation for the background and reflexes
static Color	calculateSkyInterpolation(Scene& scene, Ray& ray)
{
	static double	skyLine = scene.getSkyline();

	Vector3	normalizedDirection = normalize(ray.getDirection());

	double temp = skyLine * (-normalizedDirection.getY() + 1.0);

	return ((Color(1.0, 1.0, 1.0) * (1.0 - temp)) + (Color(0.5, 0.7, 1.0) * temp));
}

// Calculates the light rays bounce/reflection direction
static void	calculateLightRayBounceDirection(Ray& ray, Color& color)
{
	if (ray.hitRecord.material.getMetallic() == 1.0)
	{
		ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal) + (randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
		return;
	}

	if (ray.hitRecord.material.getIsDielectric() == true)
	{
		Vector3	refractedVector;
		Vector3	outwardsNormal;
		double	reflectionProbability;
		double	cosine;
		double	refractiveIndex = RI_GLASS;
		double	directionNormalDot = dot(ray.getDirection(), ray.hitRecord.normal);

		if (directionNormalDot > 0.0)
		{
			outwardsNormal = ray.hitRecord.normal * -1.0;
			cosine = refractiveIndex * directionNormalDot / vectorLength(ray.getDirection());
		}
		else
		{
			outwardsNormal = ray.hitRecord.normal;
			refractiveIndex = 1.0 / refractiveIndex;
			cosine = -directionNormalDot / vectorLength(ray.getDirection());
		}

		if (refract(ray.getDirection(), outwardsNormal, refractiveIndex, refractedVector))
		{
			reflectionProbability = schlick(cosine, refractiveIndex);
		}
		else
		{
			reflectionProbability = 1.0;
		}

		if (randomdouble() < reflectionProbability)
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal));
		}
		else
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(refractedVector);
		}

		color = Color(1.0, 1.0, 1.0);
		return;
	}

	Vector3	newTarget = ray.hitRecord.position + ray.hitRecord.normal + randomPointInsideUnitSphere();
	if (ray.hitRecord.material.getMetallic() == 0.0)
	{
		ray.setDirection(newTarget - ray.hitRecord.position);
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
		return;
	}

	if (randomdouble() < ray.hitRecord.material.getMetallic())
	{
		ray.setDirection(reflect(ray.getDirection(), ray.hitRecord.normal) + (randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
	}
	else
	{
		ray.setDirection(newTarget - ray.hitRecord.position);
	}
}

// Checks if 'ray' hits objects present 'scene'. On hit, sets 'pixelColor' to the hitted object's material color
static bool	checkHits(Scene& scene, Ray& ray)
{
	bool	anyHit = false;
	double	currentClosestObject = T_MAX;
	static std::vector<std::shared_ptr<Hittable>> hittables = scene.getHittables();

	for (std::shared_ptr<Hittable> hittable : hittables)
	{
		if (hittable->hit(ray, currentClosestObject))
		{
			if (ray.hitRecord.t0 > T_MIN)
			{
				currentClosestObject = ray.hitRecord.t0;
				anyHit = true;
			}
		}
	}

	return (anyHit);
}
