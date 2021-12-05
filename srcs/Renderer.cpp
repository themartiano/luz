#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Clock.hpp"
#include "RefractiveIndexes.hpp"
#include "SystemSpecifics.hpp"
#include "Atmosphere.hpp"
#include "Forms/Sphere.hpp"
#include "SkyTypes.hpp"
#include <cmath>
#include <iostream>
#include <thread>
#include <future>
#include <stdlib.h>
#include <unistd.h>

// Static function prototypes
static void		renderInternal(Scene& scene, int x, int y);
static Color	calculatePixelColor(Scene& scene, int x, int y);
static bool		checkHits(Scene& scene, Ray& ray);
static Color	calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
static void		calculateLightRayBounceDirection(Ray& ray, Color& color);
static Color	computeAtmosphereColor(Scene& scene, Ray& ray);
static Color	calculateSkyInterpolation(Scene& scene, Ray& ray);

// Renders the image using all the information present on 'scene'. (Objects, cameras, lights, settings, etc)
void	render(Scene& scene)
{
	std::cout << CLR_YELLOW << "Rendering..." << CLR_RESET << std::endl;

	Clock	clock;
	static int	height = scene.getYResolution();
	static int	width = scene.getXResolution();
	static int	pixelTotal = width * height;

	static unsigned int	threadCount = CORE_COUNT * THREAD_MULTIPLIER;
	volatile std::atomic<int> currentRenderPixel(0);
	std::vector<std::future<void>> futureVector;

	// Creates threads.
	for (unsigned int i = 0; i < threadCount; i++)
	{
		futureVector.push_back(
			std::async([&scene, &currentRenderPixel]()
			{
				while (true)
				{
					int index = currentRenderPixel++;
					if (index >= pixelTotal)
					{
						break;
					}

					int	x = index % width;
					int	y = index / width;

					renderInternal(scene, x, y);
				}
			}
		));
	}

	// Outputs progress using the main thread until the render is complete.
	while (true)
	{
		int localRenderPixel = currentRenderPixel;
		if (localRenderPixel >= pixelTotal)
		{
			break;
		}

		int percentage = (double(localRenderPixel) / double(pixelTotal)) * 100.0;
		std::cout << CLR_WHITE << "\r[ " << percentage << "% ]" << std::flush;

		usleep(42 * 1000); // 42ms ~~~ (42 milliseconds * 1000 microseconds)
	}

	double elapsedS = clock.stop();
	std::cout << CLR_WHITE << "\r[ 100% ]";
	std::cout << CLR_GREEN_BRIGHT << "\nRender done! " << CLR_BLUE_BRIGHT << "(Duration: " << CLR_WHITE << elapsedS << "s" << CLR_BLUE_BRIGHT << ")\n\n" << CLR_RESET;
}

// Renders the pixel (X, Y)
static void	renderInternal(Scene& scene, int x, int y)
{
	static int	width = scene.getXResolution();
	static int	sampleCount = scene.getSampleCount();
	static bool	gammaCorrected = scene.getGammaCorrected();

	Color pixelColor(0.0, 0.0, 0.0);

	for (int samples = 0; samples < sampleCount; samples++)
	{
		pixelColor += calculatePixelColor(scene, x, y);
	}
	pixelColor /= sampleCount;
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

// Calculates the color for the pixel at 'x' and 'y'. Creates rays, checks for intersections with objects on 'scene' and bounce light rays
static Color	calculatePixelColor(Scene& scene, int x, int y)
{
	static double width = double(scene.getXResolution());
	static double height = double(scene.getYResolution());

	double xU = double(x + randomDouble()) / (width - 1);
	double yV = double(y + randomDouble()) / (height - 1);

	static Vector3	cameraPosition = scene.getActiveCamera().getPosition();
	static Vector3	cameraLookDirection = scene.getActiveCamera().getDirection();

    static double	viewportWidth = 2.0 * tan((((double)scene.getActiveCamera().getFOV() * M_PI) / 180.0) / 2.0);
    static double	viewportHeight = (height / width) * viewportWidth;

	static double	lensRadius = scene.getActiveCamera().getAperture() / 2.0;
	static double	focusDistance = scene.getActiveCamera().getFocusDistance();

	static Vector3	w = Utilities::normalize(cameraLookDirection);
	static Vector3	viewUp(0.0, 1.0, 0.0);
	static Vector3	u = Utilities::normalize(Utilities::cross(viewUp, w));
	static Vector3	v = Utilities::cross(w, u);

	static Vector3	horizontal = u * viewportWidth * focusDistance;
	static Vector3	vertical = v * viewportHeight * focusDistance;
	static Vector3	lowerLeftCorner = cameraPosition - (horizontal / 2.0) - (vertical / 2.0) - (w * focusDistance);

	Vector3	offset(0.0, 0.0, 0.0);
	if (lensRadius > 0.0)
	{
		Vector3	rd = Utilities::randomPointInsideUnitDisk() * lensRadius;
		offset = u * rd.getX() + v * rd.getY();
	}

	Ray ray(cameraPosition + offset, lowerLeftCorner + (horizontal * xU) + (vertical * yV) - cameraPosition - offset);

	return (calculateLightRaysColor(ray, scene, 0));
}

// Properly calculates light rays bounces, reflections, etc and returns the resulting color
static Color	calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	static int		maxLightBounces = scene.getMaxLightBounces();
	static short	skyType = scene.getRenderSky();
	static Color	staticBackgroundColor = scene.getBackgroundColor();

	Color color, emitted = Color(0.0, 0.0, 0.0);
	if (bounces > maxLightBounces)
	{
		return (color);
	}

	if (checkHits(scene, ray))
	{
		//ray.setOrigin(ray.hitRecord.position + (ray.hitRecord.normal * T_MIN));
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

		if ((ray.hitRecord.material.getMetallic() == 1.0 && Utilities::dot(ray.getDirection(), ray.hitRecord.normal) <= 0.0))
		{
			return (emitted + color);
		}

		return ((Color(0.0, 0.0, 0.00001) * ray.hitRecord.t0) + emitted + color * calculateLightRaysColor(ray, scene, bounces + 1));
	}

	if (skyType == SKY_ATMOSPHERE)
	{
		return (computeAtmosphereColor(scene, ray));
	}
	else if (skyType == SKY_LINEAR)
	{
		return (calculateSkyInterpolation(scene, ray));
	}
	else
	{
		return (staticBackgroundColor);
	}
}

// Computes the atmosphere color
static Color	computeAtmosphereColor(Scene& scene, Ray& ray)
{
	// If the Earth radius is not added, the origin will be inside the Earth
	Ray atmosphereRay(ray.getOrigin(), Utilities::normalize(ray.getDirection()));

	double t_max = T_MAX;

	// Checks ray collisions with Earth
	if (planetaryHit(scene.getAtmosphere().getEarthRadius(), atmosphereRay) && atmosphereRay.hitRecord.t1 > 0.0)
	{
		t_max = std::max(0.0, atmosphereRay.hitRecord.t0);
	}

	Color atmosphereColor = scene.getAtmosphere().computeIncidentLight(atmosphereRay, t_max);

	double random = randomDouble(0.0, 1.0);
	if (random >= 0.9996)
	{
		double diff = randomDouble(scene.getAtmosphere().getStarsBrightness() - 0.2, scene.getAtmosphere().getStarsBrightness() + 0.2) - ((atmosphereColor.getRed() + atmosphereColor.getGreen() + atmosphereColor.getBlue()) / 3.0);
		if (diff < 0.0)
		{
			diff = 0.0;
		}
		else if (diff > 1.0)
		{
			diff = 1.0;
		}
		atmosphereColor += Color(diff, diff, diff);
	}
	return (atmosphereColor);
}

// Calculates the sky interpolation for the background and reflexes
static Color	calculateSkyInterpolation(Scene& scene, Ray& ray)
{
	static double	skyLine = scene.getSkyline();

	Vector3	normalizedDirection = Utilities::normalize(ray.getDirection());

	double temp = skyLine * (normalizedDirection.getY() + 1.0);

	return ((Color(1.0, 1.0, 1.0) * (1.0 - temp)) + (Color(0.5, 0.7, 1.0) * temp));
}

// Calculates the light rays bounce/reflection direction
static void	calculateLightRayBounceDirection(Ray& ray, Color& color)
{
	if (ray.hitRecord.material.getMetallic() == 1.0)
	{
		ray.setDirection(Utilities::reflect(ray.getDirection(), ray.hitRecord.normal) + (Utilities::randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
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
		double	directionNormalDot = Utilities::dot(ray.getDirection(), ray.hitRecord.normal);

		if (directionNormalDot > 0.0)
		{
			outwardsNormal = ray.hitRecord.normal * -1.0;
			cosine = refractiveIndex * directionNormalDot / Utilities::vectorLength(ray.getDirection());
		}
		else
		{
			outwardsNormal = ray.hitRecord.normal;
			refractiveIndex = 1.0 / refractiveIndex;
			cosine = -directionNormalDot / Utilities::vectorLength(ray.getDirection());
		}

		if (Utilities::refract(ray.getDirection(), outwardsNormal, refractiveIndex, refractedVector))
		{
			reflectionProbability = Utilities::schlick(cosine, refractiveIndex);
		}
		else
		{
			reflectionProbability = 1.0;
		}

		if (randomDouble() < reflectionProbability)
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(Utilities::reflect(ray.getDirection(), ray.hitRecord.normal));
		}
		else
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(refractedVector);
		}

		color = Color(1.0, 1.0, 1.0);
		return;
	}

	Vector3	newTarget = ray.hitRecord.position + ray.hitRecord.normal + Utilities::randomPointInsideUnitSphere();
	if (ray.hitRecord.material.getMetallic() == 0.0)
	{
		ray.setDirection(newTarget - ray.hitRecord.position);
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
		return;
	}

	if (randomDouble() < ray.hitRecord.material.getMetallic())
	{
		ray.setDirection(Utilities::reflect(ray.getDirection(), ray.hitRecord.normal) + (Utilities::randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
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
	static	std::vector<std::shared_ptr<Hittable>> hittables = scene.getHittables();

	for (std::shared_ptr<Hittable> hittable : hittables)
	{
		if (hittable->hit(ray, currentClosestObject))
		{
			// if (ray.hitRecord.t0 > T_MIN)
			// {
				currentClosestObject = ray.hitRecord.t0;
				anyHit = true;
			// }
		}
	}

	return (anyHit);
}
