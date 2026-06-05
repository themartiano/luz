#include "RendererInternal.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "SkyTypes.hpp"
#include "Random.hpp"
#include <cmath>
#include <memory>
#include <vector>
#include <algorithm>

namespace
{
	constexpr int		RUSSIAN_ROULETTE_START_BOUNCE = 3;
	constexpr double	PATH_THROUGHPUT_EPSILON = 1e-6;
	constexpr double	RUSSIAN_ROULETTE_MIN_SURVIVAL = 0.05;
	constexpr double	RUSSIAN_ROULETTE_MAX_SURVIVAL = 0.95;

	double	scatterPDFValue(const ScatterRecord& scatterRecord, const Vector3& direction)
	{
		switch (scatterRecord.pdfType)
		{
			case SCATTER_PDF_COSINE:
			{
				double cosine = Utilities::dot(direction, scatterRecord.cosineBasis.getW());

				return ((cosine <= 0.0) ? 0.0 : cosine / D_PI);
			}
			case SCATTER_PDF_SPHERE:
				return (1.0 / (4.0 * D_PI));
			default:
				return (0.0);
		}
	}

	Vector3	scatterPDFGenerate(const ScatterRecord& scatterRecord)
	{
		switch (scatterRecord.pdfType)
		{
			case SCATTER_PDF_COSINE:
				return (scatterRecord.cosineBasis.local(randomEngine.cosineDirection()));
			case SCATTER_PDF_SPHERE:
				return (randomEngine.pointInsideUnitSphere());
			default:
				return (Vector3(0.0, 0.0, 0.0));
		}
	}

	double	lightPDFValue(const std::vector<std::shared_ptr<Hittable>>& lights, const Vector3& origin, const Vector3& direction)
	{
		if (lights.size() == 1)
		{
			return (lights[0]->pdfValue(origin, direction));
		}

		double sum = 0.0;

		for (const std::shared_ptr<Hittable>& light : lights)
		{
			sum += light->pdfValue(origin, direction);
		}

		return (sum / lights.size());
	}

	Vector3	lightPDFGenerate(const std::vector<std::shared_ptr<Hittable>>& lights, const Vector3& origin)
	{
		if (lights.size() == 1)
		{
			return (lights[0]->random(origin));
		}

		const unsigned int randomIndex = randomEngine.integer(0, lights.size() - 1);

		return (lights[randomIndex]->random(origin));
	}

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

	double	maxChannel(const Color& color)
	{
		return (std::max(color.getRed(), std::max(color.getGreen(), color.getBlue())));
	}

	bool	isTerminatedThroughput(const Color& throughput)
	{
		const double throughputMax = maxChannel(throughput);

		return (!std::isfinite(throughputMax) || throughputMax <= PATH_THROUGHPUT_EPSILON);
	}

	bool	applyRussianRoulette(Color& throughput, int bounces)
	{
		if (bounces < RUSSIAN_ROULETTE_START_BOUNCE)
		{
			return (true);
		}

		double survivalProbability = maxChannel(throughput);
		survivalProbability = std::max(
			RUSSIAN_ROULETTE_MIN_SURVIVAL,
			std::min(RUSSIAN_ROULETTE_MAX_SURVIVAL, survivalProbability)
		);
		if (randomEngine.doubleFloat() > survivalProbability)
		{
			return (false);
		}
		throughput /= survivalProbability;

		return (true);
	}
}

Color	Renderer::internal::_calculatePixelColor(Scene& scene, const RenderCamera& renderCamera, std::size_t x, std::size_t y)
{
	Ray	ray = internal::_generateRay(renderCamera, x, y);

	return(
		internal::_calculateLightRaysColor(ray, scene)
	);
}

// Properly calculates light rays bounces, reflections, refractions, intersection, etc and returns the resulting color
Color	Renderer::internal::_calculateLightRaysColor(const Ray& ray, Scene& scene)
{
	const int		maxLightBounces = scene.getMaxLightBounces();
	const auto		skyType = scene.getRenderSky();
	const Color		backgroundColor = scene.getBackgroundColor();
	const auto&		lights = scene.getLights();
	const auto		lightCount = lights.size();
	//static bool		distanceBlueness = scene.getDistanceBlueness();

	Ray		currentRay = ray;
	Color	accumulatedColor(0.0, 0.0, 0.0);
	Color	throughput(1.0, 1.0, 1.0);

	for (int bounces = 0; bounces <= maxLightBounces; bounces++)
	{
		HitRecord	hitRecord;
		if (!_checkHits(scene, currentRay, hitRecord))
		{
			Color	skyColor;
			switch(skyType)
			{
				case (SKY_ATMOSPHERE):
					skyColor = _computeAtmosphereColor(scene, currentRay);
					break;
				case (SKY_LINEAR):
					skyColor = _calculateSkyInterpolation(scene, currentRay);
					break;
				default:
					skyColor = backgroundColor;
					break;
			}

			return (accumulatedColor + clampRayColor(throughput * skyColor));
		}

		ScatterRecord	scatterRecord;
		Color emitted = hitRecord.material->emitted();

		if (!hitRecord.material->scatter(currentRay, hitRecord, scatterRecord))
		{
			return (accumulatedColor + clampRayColor(throughput * emitted));
		}

		if (scatterRecord.isSpecular)
		{
			throughput = clampRayColor(throughput * scatterRecord.attenuation);
			if (isTerminatedThroughput(throughput) || !applyRussianRoulette(throughput, bounces))
			{
				return (accumulatedColor);
			}
			currentRay = scatterRecord.specularRay;
			continue;
		}

		accumulatedColor += throughput * emitted;
		if (isTerminatedThroughput(throughput * scatterRecord.attenuation))
		{
			return (accumulatedColor);
		}

		double	pdfValue;
		Ray		scattered;
		if (lightCount > 0)
		{
			Vector3 scatteredDirection;
			if (randomEngine.doubleFloat() < 0.5)
			{
				scatteredDirection = lightPDFGenerate(lights, hitRecord.position);
			}
			else
			{
				scatteredDirection = scatterPDFGenerate(scatterRecord);
			}
			scattered = Ray(hitRecord.position, scatteredDirection);
			pdfValue = 0.5 * lightPDFValue(lights, hitRecord.position, scattered.getDirection())
				+ 0.5 * scatterPDFValue(scatterRecord, scattered.getDirection());
		}
		else
		{
			scattered = Ray(hitRecord.position, scatterPDFGenerate(scatterRecord));
			pdfValue = scatterPDFValue(scatterRecord, scattered.getDirection());
		}

		if (pdfValue <= 0.0 || !std::isfinite(pdfValue))
		{
			return (accumulatedColor);
		}

		throughput = clampRayColor(
			throughput *
			scatterRecord.attenuation *
			hitRecord.material->scatteringPDF(scattered, hitRecord) /
			pdfValue
		);
		if (isTerminatedThroughput(throughput) || !applyRussianRoulette(throughput, bounces))
		{
			return (accumulatedColor);
		}
		currentRay = scattered;
	}

	return (accumulatedColor);
}
