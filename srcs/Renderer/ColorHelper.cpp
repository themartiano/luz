#include "Renderer/Renderer.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "PDFs/HittablePDF.hpp"
#include "PDFs/CosinePDF.hpp"
#include "PDFs/MixturePDF.hpp"
#include "SkyTypes.hpp"
#include <cmath>

// Calculates the color for the pixel at 'x' and 'y'. Creates rays, checks for intersections with objects on 'scene' and bounce light rays
Color	Renderer::internal::_calculatePixelColor(Scene& scene, int x, int y)
{
	static double width = double(scene.getXResolution());
	static double height = double(scene.getYResolution());

	double xU = double(x + Utilities::randomDouble()) / (width - 1);
	double yV = double(y + Utilities::randomDouble()) / (height - 1);

	static Vector3	cameraPosition = scene.getActiveCamera().getPosition();
	static Vector3	cameraLookDirection = scene.getActiveCamera().getDirection();

	static double	viewportWidth = 2.0 * tan(((scene.getActiveCamera().getFOV() * D_PI) / 180.0) / 2.0);
	static double	viewportHeight = (height / width) * viewportWidth;

	static double	lensRadius = scene.getActiveCamera().getAperture() / 2.0;
	static double	focusDistance = scene.getActiveCamera().getFocusDistance();

	static Vector3	w = Utilities::normalize(cameraLookDirection);
	static Vector3	viewUp(0.0, 1.0, 0.0);
	static Vector3	u = Utilities::normalize(Utilities::cross(viewUp, w));
	static Vector3	v = Utilities::cross(w, u);

	static Vector3	horizontal = u * viewportWidth * focusDistance;
	static Vector3	vertical = v * viewportHeight * focusDistance;
	static Vector3	lowerLeftCorner = cameraPosition + (horizontal / 2.0) + (vertical / 2.0) + (w * focusDistance);

	Vector3	offset(0.0, 0.0, 0.0);
	if (lensRadius > 0.0)
	{
		Vector3	rd = Utilities::randomPointInsideUnitDisk() * lensRadius;
		offset = u * rd.getX() + v * rd.getY();
	}

	Ray ray(cameraPosition + offset, lowerLeftCorner - (horizontal * xU) - (vertical * yV) - cameraPosition - offset);

	return (_calculateLightRaysColor(ray, scene, 0));
}

// Properly calculates light rays bounces, reflections, etc and returns the resulting color
Color	Renderer::internal::_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	static int		maxLightBounces = scene.getMaxLightBounces();
	static auto		skyType = scene.getRenderSky();
	static Color	staticBackgroundColor = scene.getBackgroundColor();
	static auto		lights = scene.getLights();
	static auto		lightCount = scene.getLights().size();
	//static bool		distanceBlueness = scene.getDistanceBlueness();

	if (bounces > maxLightBounces)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	HitRecord		hitRecord;
	if (!_checkHits(scene, ray, hitRecord))
	{
		switch(skyType)
		{
			case (SKY_ATMOSPHERE):
				return (_computeAtmosphereColor(scene, ray));
			case (SKY_LINEAR):
				return (_calculateSkyInterpolation(scene, ray));
			default:
				return (staticBackgroundColor);
		}
	}
	ScatterRecord	scatterRecord;

	Color emitted = hitRecord.material->emitted();

	if (!hitRecord.material->scatter(ray, hitRecord, scatterRecord))
	{
		return (emitted);
	}

	if (scatterRecord.isSpecular)
	{
		return (scatterRecord.attenuation * _calculateLightRaysColor(scatterRecord.specularRay, scene, bounces + 1));
	}

	double	pdfValue;
	Ray		scattered;
	if (lightCount > 0)
	{
		std::shared_ptr<HittablePDF> lightPDF = std::make_shared<HittablePDF>(lights, hitRecord.position);
		MixturePDF mixturePDF(lightPDF, scatterRecord.pdfPtr);
		scattered = Ray(hitRecord.position, mixturePDF.generate());
		pdfValue = mixturePDF.value(scattered.getDirection());
	}
	else
	{
		std::shared_ptr<PDF> pdf = scatterRecord.pdfPtr;
		scattered = Ray(hitRecord.position, pdf->generate());
		pdfValue = pdf->value(scattered.getDirection());
	}

	if (pdfValue < 0.5) // Clamping to fix fireflies
	{
		pdfValue = 0.5;
	}

	return (emitted + scatterRecord.attenuation * hitRecord.material->scatteringPDF(scattered, hitRecord) * _calculateLightRaysColor(scattered, scene, bounces + 1) / pdfValue);
}
