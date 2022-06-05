#include "Renderer/Renderer.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "PDFs/HittablePDF.hpp"
#include "PDFs/CosinePDF.hpp"
#include "PDFs/MixturePDF.hpp"
#include "SkyTypes.hpp"
#include "Random.hpp"
#include <cmath>

Color	Renderer::internal::_calculatePixelColor(Scene& scene, int x, int y)
{
	Ray	ray = internal::_generateRay(scene, x, y);

	return(
		internal::_calculateLightRaysColor(ray, scene, 0)
	);
}

// Properly calculates light rays bounces, reflections, refractions, intersection, etc and returns the resulting color
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
