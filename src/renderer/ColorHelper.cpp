#include "RendererInternal.hpp"
#include "Renderer/CausticPhotonMap.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "SkyTypes.hpp"
#include "Sampler.hpp"
#include <cmath>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstdint>

namespace
{
	constexpr int		RUSSIAN_ROULETTE_START_BOUNCE = 3;
	constexpr double	PATH_THROUGHPUT_EPSILON = 1e-6;
	constexpr double	RUSSIAN_ROULETTE_MIN_SURVIVAL = 0.05;
	constexpr double	RUSSIAN_ROULETTE_MAX_SURVIVAL = 0.95;
	constexpr int		DIRECT_LIGHT_FULL_SAMPLE_BOUNCES = 2;
	constexpr double	DIRECT_LIGHT_LATE_SAMPLE_PROBABILITY = 0.5;

	struct	LightDistribution
	{
		const std::vector<double>*	cumulativeWeights = nullptr;
		double						totalWeight = 0.0;
	};

	struct	LightSample
	{
		Vector3	direction;
		Color	emitted;
		double	pdf = 0.0;
		double	tMax = 0.0;
		bool	valid = false;
	};

	double	scatterPDFValue(
		const ScatterRecord& scatterRecord,
		const HitRecord& hitRecord,
		const Vector3& direction
	)
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
			case SCATTER_PDF_HENYEY_GREENSTEIN:
			{
				const double directionLengthSquared = Utilities::vectorLengthSquared(direction);
				const double phaseDirectionLengthSquared = Utilities::vectorLengthSquared(scatterRecord.phaseDirection);
				if (
					directionLengthSquared <= 0.0
					|| phaseDirectionLengthSquared <= 0.0
					|| !std::isfinite(directionLengthSquared)
					|| !std::isfinite(phaseDirectionLengthSquared)
				)
				{
					return (0.0);
				}

				const Vector3 normalizedDirection = direction / std::sqrt(directionLengthSquared);
				const Vector3 phaseDirection = scatterRecord.phaseDirection / std::sqrt(phaseDirectionLengthSquared);
				const double cosTheta = std::max(-1.0, std::min(1.0, Utilities::dot(normalizedDirection, phaseDirection)));
				const double g = std::max(-0.99, std::min(0.99, scatterRecord.phaseAnisotropy));
				const double g2 = g * g;
				const double denominator = std::max(1e-12, 1.0 + g2 - (2.0 * g * cosTheta));

				return ((1.0 - g2) / (4.0 * D_PI * denominator * std::sqrt(denominator)));
			}
			case SCATTER_PDF_BSDF:
				if (!scatterRecord.bsdfMaterial)
				{
					return (0.0);
				}
				return (scatterRecord.bsdfMaterial->scatteringPDF(
					scatterRecord.incidentRay,
					hitRecord,
					direction
				));
			default:
				return (0.0);
		}
	}

	Color	scatterBSDFCos(
		const ScatterRecord& scatterRecord,
		const HitRecord& hitRecord,
		const Vector3& direction
	)
	{
		if (scatterRecord.pdfType == SCATTER_PDF_BSDF)
		{
			if (!scatterRecord.bsdfMaterial)
			{
				return (Color(0.0, 0.0, 0.0));
			}
			return (scatterRecord.bsdfMaterial->evaluateBSDFCos(
				scatterRecord.incidentRay,
				hitRecord,
				direction
			));
		}
		return (scatterRecord.attenuation * scatterPDFValue(scatterRecord, hitRecord, direction));
	}

	Vector3	henyeyGreensteinDirection(const ScatterRecord& scatterRecord)
	{
		const double phaseDirectionLengthSquared = Utilities::vectorLengthSquared(scatterRecord.phaseDirection);
		if (phaseDirectionLengthSquared <= 0.0 || !std::isfinite(phaseDirectionLengthSquared))
		{
			return (Sampler::sphereDirection(Sampler::DIM_BSDF_DIRECTION));
		}

		const Vector3 phaseDirection = scatterRecord.phaseDirection / std::sqrt(phaseDirectionLengthSquared);
		const double g = std::max(-0.99, std::min(0.99, scatterRecord.phaseAnisotropy));
		const Sampler::Sample2D sample = Sampler::sample2D(Sampler::DIM_BSDF_DIRECTION);
		const double phi = 2.0 * D_PI * sample.x;
		double cosTheta;

		if (std::fabs(g) < 1e-3)
		{
			cosTheta = 1.0 - (2.0 * sample.y);
		}
		else
		{
			const double remapped = (1.0 - (g * g)) / (1.0 - g + (2.0 * g * sample.y));
			cosTheta = (1.0 + (g * g) - (remapped * remapped)) / (2.0 * g);
			cosTheta = std::max(-1.0, std::min(1.0, cosTheta));
		}

		const double sinTheta = std::sqrt(std::max(0.0, 1.0 - (cosTheta * cosTheta)));
		const ONB phaseBasis(phaseDirection);

		return (phaseBasis.local(
			sinTheta * std::cos(phi),
			sinTheta * std::sin(phi),
			cosTheta
		));
	}

	Vector3	scatterPDFGenerate(const ScatterRecord& scatterRecord)
	{
		switch (scatterRecord.pdfType)
		{
			case SCATTER_PDF_COSINE:
				return (scatterRecord.cosineBasis.local(Sampler::cosineHemisphere(Sampler::DIM_BSDF_DIRECTION)));
			case SCATTER_PDF_SPHERE:
				return (Sampler::sphereDirection(Sampler::DIM_BSDF_DIRECTION));
			case SCATTER_PDF_HENYEY_GREENSTEIN:
				return (henyeyGreensteinDirection(scatterRecord));
			case SCATTER_PDF_BSDF:
				return (scatterRecord.sampledDirection);
			default:
				return (Vector3(0.0, 0.0, 0.0));
		}
	}

	bool	hasWeightedLightDistribution(const LightDistribution& lightDistribution)
	{
		return (
			lightDistribution.cumulativeWeights != nullptr
			&& !lightDistribution.cumulativeWeights->empty()
			&& lightDistribution.totalWeight > 0.0
		);
	}

	double	lightSelectionProbability(
		const LightDistribution& lightDistribution,
		const std::vector<std::shared_ptr<Hittable>>& lights,
		std::size_t lightIndex
	)
	{
		if (lights.empty())
		{
			return (0.0);
		}
		if (!hasWeightedLightDistribution(lightDistribution))
		{
			return (1.0 / static_cast<double>(lights.size()));
		}

		const std::vector<double>& cumulativeWeights = *lightDistribution.cumulativeWeights;
		const double previousWeight = lightIndex == 0 ? 0.0 : cumulativeWeights[lightIndex - 1];
		const double weight = cumulativeWeights[lightIndex] - previousWeight;
		if (weight <= 0.0 || lightDistribution.totalWeight <= 0.0)
		{
			return (0.0);
		}
		return (weight / lightDistribution.totalWeight);
	}

	std::size_t	selectLightIndex(
		const LightDistribution& lightDistribution,
		const std::vector<std::shared_ptr<Hittable>>& lights
	)
	{
		if (lights.size() <= 1)
		{
			return (0);
		}
		if (hasWeightedLightDistribution(lightDistribution))
		{
			const std::vector<double>& cumulativeWeights = *lightDistribution.cumulativeWeights;
			const double target = Sampler::sample1D(Sampler::DIM_LIGHT_SELECTION) * lightDistribution.totalWeight;
			const auto weightIt = std::upper_bound(
				cumulativeWeights.begin(),
				cumulativeWeights.end(),
				target
			);

			return (std::min<std::size_t>(
				static_cast<std::size_t>(weightIt - cumulativeWeights.begin()),
				lights.size() - 1
			));
		}
		const std::size_t randomIndex = std::min<std::size_t>(
			static_cast<std::size_t>(Sampler::sample1D(Sampler::DIM_LIGHT_SELECTION) * lights.size()),
			lights.size() - 1
		);

		return (randomIndex);
	}

	double	lightPDFValue(
		const LightDistribution& lightDistribution,
		const std::vector<std::shared_ptr<Hittable>>& lights,
		const Vector3& origin,
		const Vector3& direction
	)
	{
		if (lights.empty())
		{
			return (0.0);
		}
		if (lights.size() == 1)
		{
			return (lights[0]->pdfValue(origin, direction));
		}

		double sum = 0.0;

		for (std::size_t i = 0; i < lights.size(); i++)
		{
			sum += lightSelectionProbability(lightDistribution, lights, i) * lights[i]->pdfValue(origin, direction);
		}

		return (sum);
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

	double	powerHeuristic(double firstPDF, double secondPDF)
	{
		const double first = firstPDF * firstPDF;
		const double second = secondPDF * secondPDF;
		const double sum = first + second;

		if (sum <= 0.0 || !std::isfinite(sum))
		{
			return (0.0);
		}
		return (first / sum);
	}

	bool	isTerminatedThroughput(const Color& throughput)
	{
		const double throughputMax = maxChannel(throughput);

		return (!std::isfinite(throughputMax) || throughputMax <= PATH_THROUGHPUT_EPSILON);
	}

	bool	hasEnvironmentLight(Scene& scene)
	{
		return (
			scene.hasEnvironmentMap()
			&& scene.getEnvironmentLighting()
			&& scene.getEnvironmentStrength() > 0.0
		);
	}

	bool	hasVisibleEnvironment(Scene& scene)
	{
		return (scene.hasEnvironmentMap() && scene.getEnvironmentStrength() > 0.0);
	}

	Color	sampleEnvironmentRadiance(Scene& scene, const Vector3& direction)
	{
		if (!hasVisibleEnvironment(scene))
		{
			return (Color(0.0, 0.0, 0.0));
		}
		return (
			scene.getEnvironmentMap()->sampleDirection(
				direction,
				scene.getEnvironmentRotation()
			) * scene.getEnvironmentStrength()
		);
	}

	Color	sampleProceduralStars(const Atmosphere& atmosphere, const Color& atmosphereColor)
	{
		double random = Sampler::sample1D(Sampler::DIM_ATMOSPHERE);
		if (random < 0.9996)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		double starSample = Sampler::sample1D(Sampler::DIM_ATMOSPHERE + 1);
		double diff = (atmosphere.getStarsBrightness() - 0.2 + (0.4 * starSample))
			- ((atmosphereColor.getRed() + atmosphereColor.getGreen() + atmosphereColor.getBlue()) / 3.0);
		if (diff < 0.0)
		{
			diff = 0.0;
		}
		else if (diff > 1.0)
		{
			diff = 1.0;
		}
		return (Color(diff, diff, diff));
	}

	Color	sampleAtmosphereSky(Scene& scene, Ray& ray, double environmentMISWeight)
	{
		const Atmosphere& atmosphere = scene.getAtmosphere();
		const AtmosphereSample atmosphereSample = atmosphere.sampleSegment(ray, T_MAX);
		Color background(0.0, 0.0, 0.0);

		if (hasVisibleEnvironment(scene))
		{
			background = sampleEnvironmentRadiance(scene, ray.getDirection()) * environmentMISWeight;
		}
		else
		{
			background = sampleProceduralStars(atmosphere, atmosphereSample.inScattering);
		}
		return (atmosphereSample.inScattering + (atmosphereSample.transmittance * background));
	}

	Color	sampleSceneSky(Scene& scene, Ray& ray)
	{
		switch(scene.getRenderSky())
		{
			case (SKY_ATMOSPHERE):
				return (sampleAtmosphereSky(scene, ray, 1.0));
			case (SKY_LINEAR):
				return (Renderer::internal::_calculateSkyInterpolation(scene, ray));
			case (SKY_ENVIRONMENT):
				if (hasVisibleEnvironment(scene))
					return (sampleEnvironmentRadiance(scene, ray.getDirection()));
				return (scene.getBackgroundColor());
			default:
				return (scene.getBackgroundColor());
		}
	}

	double	primaryAtmosphereTMax(const Atmosphere& atmosphere, const Ray& ray, double surfaceTMax)
	{
		if (!std::isfinite(surfaceTMax) || surfaceTMax <= T_MIN)
		{
			return (surfaceTMax);
		}

		HitRecord earthHitRecord;
		if (!planetaryHit(atmosphere.metersToSceneUnits(atmosphere.getEarthRadius()), ray, earthHitRecord) || earthHitRecord.t1 <= T_MIN)
		{
			return (surfaceTMax);
		}

		const double groundTMax = std::max(0.0, earthHitRecord.t0);
		if (!std::isfinite(groundTMax) || groundTMax <= T_MIN)
		{
			return (surfaceTMax);
		}
		return (std::max(surfaceTMax, groundTMax));
	}

	void	compositePrimaryAtmosphereSegment(Scene& scene, const Ray& ray, double tMax, Color& accumulatedColor, Color& throughput)
	{
		if (scene.getRenderSky() != SKY_ATMOSPHERE)
		{
			return;
		}

		const Atmosphere& atmosphere = scene.getAtmosphere();
		const AtmosphereSample atmosphereSample = atmosphere.sampleSegment(
			ray,
			primaryAtmosphereTMax(atmosphere, ray, tMax)
		);
		accumulatedColor += clampRayColor(throughput * atmosphereSample.inScattering);
		throughput = clampRayColor(throughput * atmosphereSample.transmittance);
	}

	double	environmentPDF(Scene& scene, const Vector3& direction)
	{
		if (!hasEnvironmentLight(scene))
		{
			return (0.0);
		}
		return (scene.getEnvironmentMap()->pdf(direction, scene.getEnvironmentRotation()));
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
		if (Sampler::sample1D(Sampler::DIM_RUSSIAN_ROULETTE) > survivalProbability)
		{
			return (false);
		}
		throughput /= survivalProbability;

		return (true);
	}

	Color	mediumTransmittance(const ScatterRecord& scatterRecord, double distanceMeters)
	{
		if (!scatterRecord.hasMediumAbsorption || !std::isfinite(distanceMeters) || distanceMeters <= 0.0)
		{
			return (Color(1.0, 1.0, 1.0));
		}

		return (Color(
			std::exp(-scatterRecord.mediumAbsorptionCoefficient.getRed() * distanceMeters),
			std::exp(-scatterRecord.mediumAbsorptionCoefficient.getGreen() * distanceMeters),
			std::exp(-scatterRecord.mediumAbsorptionCoefficient.getBlue() * distanceMeters)
		));
	}

	Color	effectiveScatterAttenuation(
		Scene& scene,
		const HitRecord& hitRecord,
		const ScatterRecord& scatterRecord
	)
	{
		if (hitRecord.frontFace)
		{
			return (scatterRecord.attenuation);
		}
		return (
			scatterRecord.attenuation
			* mediumTransmittance(scatterRecord, scene.sceneUnitsToMeters(hitRecord.t0))
		);
	}

	double	normalizedColorChannel(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		return (std::min(1.0, std::log1p(value) / std::log1p(D_MAX_RAY_COLOR_LUMINANCE)));
	}

	void	setPrimaryCoordinates(
		Denoise::FeatureVector& features,
		const Renderer::internal::RenderCamera& renderCamera,
		std::size_t x,
		std::size_t y
	)
	{
		features[0] = renderCamera.width > 1.0 ? static_cast<double>(x) / (renderCamera.width - 1.0) : 0.0;
		features[1] = renderCamera.height > 1.0 ? static_cast<double>(y) / (renderCamera.height - 1.0) : 0.0;
	}

	Denoise::FeatureVector	primaryMissFeatures(
		Scene& scene,
		const Renderer::internal::RenderCamera& renderCamera,
		Ray& ray,
		std::size_t x,
		std::size_t y
	)
	{
		Denoise::FeatureVector features;
		const Color background = sampleSceneSky(scene, ray);

		setPrimaryCoordinates(features, renderCamera, x, y);
		features[2] = 0.0;
		features[3] = 1.0;
		features[4] = 0.5;
		features[5] = 0.5;
		features[6] = 0.5;
		features[7] = normalizedColorChannel(background.getRed());
		features[8] = normalizedColorChannel(background.getGreen());
		features[9] = normalizedColorChannel(background.getBlue());
		features[10] = 0.0;
		return (features);
	}

	Denoise::FeatureVector	primaryHitFeatures(
		const HitRecord& hitRecord,
		const Renderer::internal::RenderCamera& renderCamera,
		std::size_t x,
		std::size_t y
	)
	{
		Denoise::FeatureVector features;

		Vector3 normal = hitRecord.normal;
		if (Utilities::vectorLengthSquared(normal) > 0.0)
		{
			normal = Utilities::normalize(normal);
		}
		const Color albedo = hitRecord.material ? hitRecord.material->colorAt(hitRecord) : Color(0.0, 0.0, 0.0);
		const double materialType = hitRecord.material
			? static_cast<double>(hitRecord.material->getType()) / static_cast<double>(PRINCIPLED)
			: 0.0;

		setPrimaryCoordinates(features, renderCamera, x, y);
		features[2] = 1.0;
		features[3] = hitRecord.t0 > 0.0 ? hitRecord.t0 / (1.0 + hitRecord.t0) : 0.0;
		features[4] = normal.getX() * 0.5 + 0.5;
		features[5] = normal.getY() * 0.5 + 0.5;
		features[6] = normal.getZ() * 0.5 + 0.5;
		features[7] = normalizedColorChannel(albedo.getRed());
		features[8] = normalizedColorChannel(albedo.getGreen());
		features[9] = normalizedColorChannel(albedo.getBlue());
		features[10] = materialType;
		return (features);
	}

	bool	isShadowOccluded(Scene& scene, Ray& shadowRay, double tMax)
	{
		if (tMax <= T_MIN)
		{
			return (false);
		}

		const auto& accelerationStructure = scene.getAccelerationStructure();

		if (accelerationStructure && accelerationStructure->hitAny(shadowRay, T_MIN, tMax))
		{
			return (true);
		}

		const auto& unacceleratedHittables = scene.getUnacceleratedHittables();
		const auto& hittables = (!accelerationStructure && unacceleratedHittables.empty())
			? scene.getHittables()
			: unacceleratedHittables;

		for (const std::shared_ptr<Hittable>& hittable : hittables)
		{
			if (hittable->hitAny(shadowRay, T_MIN, tMax))
			{
				return (true);
			}
		}

		return (false);
	}

	LightSample	sampleEnvironmentLight(Scene& scene, const Vector3& origin)
	{
		LightSample sample;

		if (!hasEnvironmentLight(scene))
		{
			return (sample);
		}

		const EnvironmentMap::Sample environmentSample = scene.getEnvironmentMap()->sample(
			Sampler::sample1D(Sampler::DIM_ENVIRONMENT_SELECTION),
			Sampler::sample2D(Sampler::DIM_ENVIRONMENT_POINT),
			scene.getEnvironmentRotation()
		);
		if (!environmentSample.valid || environmentSample.pdf <= 0.0 || !std::isfinite(environmentSample.pdf))
		{
			return (sample);
		}

		sample.direction = environmentSample.direction;
		sample.emitted = environmentSample.radiance * scene.getEnvironmentStrength();
		sample.pdf = environmentSample.pdf;
		sample.tMax = T_MAX;
		if (maxChannel(sample.emitted) <= 0.0)
		{
			return (sample);
		}

		Ray shadowRay = Ray::fromNormalizedDirection(origin, sample.direction);
		if (isShadowOccluded(scene, shadowRay, T_MAX))
		{
			return (sample);
		}

		sample.valid = true;
		return (sample);
	}

	double	directLightSampleProbability(int bounces)
	{
		if (bounces < DIRECT_LIGHT_FULL_SAMPLE_BOUNCES)
		{
			return (1.0);
		}
		return (DIRECT_LIGHT_LATE_SAMPLE_PROBABILITY);
	}

	LightSample	sampleLight(
		const LightDistribution& lightDistribution,
		const std::vector<std::shared_ptr<Hittable>>& lights,
		const Vector3& origin
	)
	{
		LightSample sample;

		if (lights.empty())
		{
			return (sample);
		}

		const std::size_t lightIndex = selectLightIndex(lightDistribution, lights);
		const double selectionProbability = lightSelectionProbability(lightDistribution, lights, lightIndex);
		if (selectionProbability <= 0.0 || !std::isfinite(selectionProbability))
		{
			return (sample);
		}

		HittableLightSample surfaceSample;
		if (!lights[lightIndex]->sampleLight(origin, surfaceSample) || !surfaceSample.valid)
		{
			return (sample);
		}

		sample.direction = surfaceSample.direction;
		sample.pdf = surfaceSample.pdf * selectionProbability;
		sample.tMax = surfaceSample.tMax;
		if (
			sample.pdf <= 0.0
			|| sample.tMax <= T_MIN
			|| !std::isfinite(sample.pdf)
			|| !std::isfinite(sample.tMax)
		)
		{
			return (sample);
		}

		Material* material = surfaceSample.material
			? surfaceSample.material
			: lights[lightIndex]->getMaterial();
		if (!material)
		{
			return (sample);
		}

		sample.emitted = surfaceSample.hasEmitted
			? surfaceSample.emitted
			: material->emitted();
		if (maxChannel(sample.emitted) <= 0.0)
		{
			return (sample);
		}

		sample.valid = true;
		return (sample);
	}

	Color	estimateDirectLighting(
		Scene& scene,
		const std::vector<std::shared_ptr<Hittable>>& lights,
		const LightDistribution& lightDistribution,
		HitRecord& hitRecord,
		const ScatterRecord& scatterRecord,
		int bounces
	)
	{
		if (lights.empty())
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const double sampleProbability = directLightSampleProbability(bounces);
		if (sampleProbability <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		if (
			sampleProbability < 1.0
			&& Sampler::sample1D(Sampler::DIM_LIGHT_STRATEGY) >= sampleProbability
		)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const LightSample lightSample = sampleLight(lightDistribution, lights, hitRecord.position);
		if (!lightSample.valid)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const double scatterSamplePDF = scatterPDFValue(scatterRecord, hitRecord, lightSample.direction);
		const Color bsdfCos = scatterBSDFCos(scatterRecord, hitRecord, lightSample.direction);
		if (
			scatterSamplePDF <= 0.0
			|| !std::isfinite(scatterSamplePDF)
			|| maxChannel(bsdfCos) <= 0.0
		)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		Ray shadowRay = Ray::fromNormalizedDirection(hitRecord.position, lightSample.direction);
		if (isShadowOccluded(scene, shadowRay, lightSample.tMax - T_MIN))
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const double misWeight = powerHeuristic(lightSample.pdf, scatterSamplePDF);

		return (
			bsdfCos
			* lightSample.emitted
			* (misWeight / (lightSample.pdf * sampleProbability))
		);
	}

	Color	estimateEnvironmentLighting(
		Scene& scene,
		HitRecord& hitRecord,
		const ScatterRecord& scatterRecord,
		int bounces
	)
	{
		const double sampleProbability = directLightSampleProbability(bounces);
		if (sampleProbability <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		if (
			sampleProbability < 1.0
			&& Sampler::sample1D(Sampler::DIM_ENVIRONMENT_STRATEGY) >= sampleProbability
		)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const LightSample lightSample = sampleEnvironmentLight(scene, hitRecord.position);
		if (!lightSample.valid)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const double scatterSamplePDF = scatterPDFValue(scatterRecord, hitRecord, lightSample.direction);
		const Color bsdfCos = scatterBSDFCos(scatterRecord, hitRecord, lightSample.direction);
		if (
			scatterSamplePDF <= 0.0
			|| !std::isfinite(scatterSamplePDF)
			|| maxChannel(bsdfCos) <= 0.0
		)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const double misWeight = powerHeuristic(lightSample.pdf, scatterSamplePDF);

		return (
			bsdfCos
			* lightSample.emitted
			* (misWeight / (lightSample.pdf * sampleProbability))
		);
	}

	Color	calculateLightRaysColor(
		const Ray& ray,
		Scene& scene,
		Denoise::FeatureVector* primaryFeatures,
		const Renderer::internal::RenderCamera* renderCamera,
		std::size_t x,
		std::size_t y
	);
}

Color	Renderer::internal::_calculatePixelColor(Scene& scene, const RenderCamera& renderCamera, std::size_t x, std::size_t y)
{
	Ray	ray = internal::_generateRay(renderCamera, x, y);

	return(
		internal::_calculateLightRaysColor(ray, scene)
	);
}

Renderer::internal::RenderSample	Renderer::internal::_calculatePixelSample(Scene& scene, const RenderCamera& renderCamera, std::size_t x, std::size_t y)
{
	Ray	ray = internal::_generateRay(renderCamera, x, y);
	RenderSample sample;

	sample.color = calculateLightRaysColor(ray, scene, &sample.features, &renderCamera, x, y);
	return (sample);
}

// Properly calculates light rays bounces, reflections, refractions, intersection, etc and returns the resulting color
Color	Renderer::internal::_calculateLightRaysColor(const Ray& ray, Scene& scene)
{
	return (calculateLightRaysColor(ray, scene, nullptr, nullptr, 0, 0));
}

namespace
{
	Color	calculateLightRaysColor(
		const Ray& ray,
		Scene& scene,
		Denoise::FeatureVector* primaryFeatures,
		const Renderer::internal::RenderCamera* renderCamera,
		std::size_t x,
		std::size_t y
	)
	{
		const int		maxLightBounces = scene.getMaxLightBounces();
		const auto		skyType = scene.getRenderSky();
		const auto&		lights = scene.getLights();
		const auto		lightCount = lights.size();
		const bool		environmentLight = hasEnvironmentLight(scene);
		const LightDistribution lightDistribution = {
			&scene.getLightSelectionCumulativeWeights(),
			scene.getLightSelectionTotalWeight()
		};
		const CausticPhotonMap* causticPhotonMap = scene.getCausticPhotonMap().get();
		//static bool		distanceBlueness = scene.getDistanceBlueness();

		Ray		currentRay = ray;
		Color	accumulatedColor(0.0, 0.0, 0.0);
		Color	throughput(1.0, 1.0, 1.0);
		bool	previousBounceSpecular = true;
		Vector3	previousScatterOrigin;
		double	previousScatterPDF = 0.0;

		for (int bounces = 0; bounces <= maxLightBounces; bounces++)
		{
			Sampler::setBounce(static_cast<std::uint32_t>(bounces));
			HitRecord	hitRecord;
			if (!Renderer::internal::_checkHits(scene, currentRay, hitRecord))
			{
				if (bounces == 0 && primaryFeatures != nullptr && renderCamera != nullptr)
				{
					*primaryFeatures = primaryMissFeatures(scene, *renderCamera, currentRay, x, y);
				}
				double environmentMISWeight = 1.0;
				if (
					(skyType == SKY_ENVIRONMENT || skyType == SKY_ATMOSPHERE)
					&& bounces > 0
					&& !previousBounceSpecular
					&& environmentLight
				)
				{
					environmentMISWeight = powerHeuristic(
						previousScatterPDF,
						environmentPDF(scene, currentRay.getDirection())
					);
				}
				Color skyColor(0.0, 0.0, 0.0);
				switch (skyType)
				{
					case SKY_ATMOSPHERE:
						skyColor = sampleAtmosphereSky(scene, currentRay, environmentMISWeight);
						break;
					case SKY_ENVIRONMENT:
						skyColor = sampleSceneSky(scene, currentRay) * environmentMISWeight;
						break;
					case SKY_LINEAR:
						skyColor = Renderer::internal::_calculateSkyInterpolation(scene, currentRay);
						break;
					default:
						skyColor = scene.getBackgroundColor();
						break;
				}

				return (accumulatedColor + clampRayColor(throughput * skyColor));
			}
			if (bounces == 0 && primaryFeatures != nullptr && renderCamera != nullptr)
			{
				*primaryFeatures = primaryHitFeatures(hitRecord, *renderCamera, x, y);
			}
			if (bounces == 0)
			{
				compositePrimaryAtmosphereSegment(scene, currentRay, hitRecord.t0, accumulatedColor, throughput);
				if (isTerminatedThroughput(throughput))
				{
					return (accumulatedColor);
				}
			}

			ScatterRecord	scatterRecord;
			Color emitted = hitRecord.material->emitted();

			if (!hitRecord.material->scatter(currentRay, hitRecord, scatterRecord))
			{
				const double previousLightPDF = (!previousBounceSpecular && lightCount > 0)
					? lightPDFValue(lightDistribution, lights, previousScatterOrigin, currentRay.getDirection())
					: 0.0;
				const double emissionMISWeight = previousBounceSpecular
					? 1.0
					: powerHeuristic(previousScatterPDF, previousLightPDF);

				return (accumulatedColor + clampRayColor((throughput * emitted) * emissionMISWeight));
			}
			const Color attenuation = effectiveScatterAttenuation(scene, hitRecord, scatterRecord);

			if (scatterRecord.isSpecular)
			{
				throughput = clampRayColor(throughput * attenuation);
				if (isTerminatedThroughput(throughput) || !applyRussianRoulette(throughput, bounces))
				{
					return (accumulatedColor);
				}
				currentRay = scatterRecord.specularRay;
				previousBounceSpecular = true;
				previousScatterPDF = 0.0;
				continue;
			}

			accumulatedColor += throughput * emitted;
			if (isTerminatedThroughput(throughput * attenuation))
			{
				return (accumulatedColor);
			}
			if (causticPhotonMap)
			{
				accumulatedColor += clampRayColor(
					throughput * causticPhotonMap->estimate(hitRecord, scatterRecord)
				);
			}

			if (lightCount > 0)
			{
				accumulatedColor += clampRayColor(
					throughput * estimateDirectLighting(
						scene,
						lights,
						lightDistribution,
						hitRecord,
						scatterRecord,
						bounces
					)
				);
			}
			if (environmentLight)
			{
				accumulatedColor += clampRayColor(
					throughput * estimateEnvironmentLighting(
						scene,
						hitRecord,
						scatterRecord,
						bounces
					)
				);
			}

			Ray scattered = Ray::fromNormalizedDirection(hitRecord.position, scatterPDFGenerate(scatterRecord));
			const double pdfValue = scatterPDFValue(scatterRecord, hitRecord, scattered.getDirection());

			if (pdfValue <= 0.0 || !std::isfinite(pdfValue))
			{
				return (accumulatedColor);
			}

			throughput = clampRayColor(
				throughput * attenuation
			);
			if (isTerminatedThroughput(throughput) || !applyRussianRoulette(throughput, bounces))
			{
				return (accumulatedColor);
			}
			currentRay = scattered;
			previousBounceSpecular = false;
			previousScatterOrigin = hitRecord.position;
			previousScatterPDF = pdfValue;
		}

		return (accumulatedColor);
	}
}
