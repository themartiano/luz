#include "Renderer/Renderer.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "PDFs/HittablePDF.hpp"
#include "PDFs/CosinePDF.hpp"
#include "PDFs/MixturePDF.hpp"
#include "SkyTypes.hpp"
#include <cmath>

// Calculates the color for the pixel at 'x' and 'y'. Creates rays, checks for intersections with objects on 'scene' and bounce light rays
Color	Renderer::_calculatePixelColor(Scene& scene, int x, int y)
{
	static double width = double(scene.getXResolution());
	static double height = double(scene.getYResolution());

	double xU = double(x + randomDouble()) / (width - 1);
	double yV = double(y + randomDouble()) / (height - 1);

	static Vector3	cameraPosition = scene.getActiveCamera().getPosition();
	static Vector3	cameraLookDirection = scene.getActiveCamera().getDirection();

    static double	viewportWidth = 2.0 * tan((((double)scene.getActiveCamera().getFOV() * D_PI) / 180.0) / 2.0);
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

	return (_calculateLightRaysColor(ray, scene, 0));
}

// Properly calculates light rays bounces, reflections, etc and returns the resulting color
Color	Renderer::_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	static int		maxLightBounces = scene.getMaxLightBounces();
	static short	skyType = scene.getRenderSky();
	static Color	staticBackgroundColor = scene.getBackgroundColor();
	static auto		lights = scene.getLights();

	Color color, emitted = Color(0.0, 0.0, 0.0);
	if (bounces > maxLightBounces)
	{
		return (color);
	}
	if (_checkHits(scene, ray))
	{
		Ray	oldRay = ray; // make const

		//ray.setOrigin(ray.hitRecord.position + (ray.hitRecord.normal * T_MIN));
		ray.setOrigin(ray.hitRecord.position);

		if (ray.hitRecord.material.getIsEmissive() == true)
		{
			emitted = ray.hitRecord.material.getColor() * ray.hitRecord.material.getLightIntensity();
			return (emitted);
		}

		std::shared_ptr<HittablePDF> lightPDF = std::make_shared<HittablePDF>(lights, oldRay.hitRecord.position);
		std::shared_ptr<CosinePDF> cosinePDF = std::make_shared<CosinePDF>(oldRay.hitRecord.normal);
		MixturePDF mixturePDF(lightPDF, cosinePDF);

		_calculateLightRayBounceDirection(ray, color, mixturePDF);

		if ((ray.hitRecord.material.getMetallic() == 1.0 && Utilities::dot(ray.getDirection(), ray.hitRecord.normal) <= 0.0))
		{
			return (emitted + color);
		}

		Color	blueness;
		if (scene.getDistanceBlueness())
		{
			blueness = Color(0.0, 0.0, 0.00001 * ray.hitRecord.t0);
		}
		else
		{
			blueness = Color(0.0, 0.0, 0.0);
		}
		double pdfValue = mixturePDF.value(ray.getDirection());
		return (blueness + emitted + color * Utilities::scatteringPDF(oldRay, ray) * _calculateLightRaysColor(ray, scene, bounces + 1) / pdfValue);
	}

	if (skyType == SKY_ATMOSPHERE)
	{
		return (_computeAtmosphereColor(scene, ray));
	}
	else if (skyType == SKY_LINEAR)
	{
		return (_calculateSkyInterpolation(scene, ray));
	}
	else
	{
		return (staticBackgroundColor);
	}
}
