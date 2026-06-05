#include "RendererInternal.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "PDFs/HittablePDF.hpp"
#include "PDFs/CosinePDF.hpp"
#include "PDFs/MixturePDF.hpp"
#include "SkyTypes.hpp"
#include "Random.hpp"
#include <cmath>

namespace
{
	Color	clampRayColor(Color color)
	{
		const double luminance = Utilities::luminance(color);

		if (!std::isfinite(luminance))
		{
			return (Color(0.0, 0.0, 0.0));
		}
		if (luminance > D_MAX_RAY_COLOR_LUMINANCE)
		{
			color = color * (D_MAX_RAY_COLOR_LUMINANCE / luminance);
		}

		return (color);
	}
}

Color	Renderer::internal::_calculatePixelColor(Scene& scene, std::size_t x, std::size_t y)
{
	Ray	ray = internal::_generateRay(scene, x, y);

	return(
		internal::_calculateLightRaysColor(ray, scene, 0)
	);
}

// Properly calculates light rays bounces, reflections, refractions, intersection, etc and returns the resulting color
Color	Renderer::internal::_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces)
{
	const int		maxLightBounces = scene.getMaxLightBounces();
	const auto		skyType = scene.getRenderSky();
	const Color		backgroundColor = scene.getBackgroundColor();
	const auto&		lights = scene.getLights();
	const auto		lightCount = lights.size();
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
				return (backgroundColor);
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
		return (clampRayColor(scatterRecord.attenuation * _calculateLightRaysColor(scatterRecord.specularRay, scene, bounces + 1)));
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

	if (pdfValue <= 0.0 || !std::isfinite(pdfValue))
	{
		return (emitted);
	}

	const Color contribution = scatterRecord.attenuation * hitRecord.material->scatteringPDF(scattered, hitRecord) * _calculateLightRaysColor(scattered, scene, bounces + 1) / pdfValue;

	return (emitted + clampRayColor(contribution));
}
