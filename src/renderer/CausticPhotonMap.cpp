#include "Renderer/CausticPhotonMap.hpp"
#include "RendererInternal.hpp"
#include "AABB.hpp"
#include "Defaults.hpp"
#include "Hittables/DirectionalLight.hpp"
#include "Materials/Principled.hpp"
#include "ONB.hpp"
#include "Sampler.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace
{
	constexpr std::uint32_t	CAUSTIC_SEED_SCRAMBLE = 0x6a09e667u;
	constexpr std::uint32_t	CAUSTIC_STREAM = 0x13579bdfu;
	constexpr int			RUSSIAN_ROULETTE_START_BOUNCE = 3;
	constexpr double		RUSSIAN_ROULETTE_MIN_SURVIVAL = 0.05;
	constexpr double		RUSSIAN_ROULETTE_MAX_SURVIVAL = 0.95;

	double	maxChannel(const Color& color)
	{
		return (std::max(color.getRed(), std::max(color.getGreen(), color.getBlue())));
	}

	bool	validPositiveColor(const Color& color)
	{
		return (
			std::isfinite(color.getRed())
			&& std::isfinite(color.getGreen())
			&& std::isfinite(color.getBlue())
			&& maxChannel(color) > 0.0
		);
	}

	double	phasePDF(const ScatterRecord& scatterRecord, const Vector3& direction)
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
		const double cosTheta = std::clamp(Utilities::dot(normalizedDirection, phaseDirection), -1.0, 1.0);
		const double g = std::clamp(scatterRecord.phaseAnisotropy, -0.99, 0.99);
		const double g2 = g * g;
		const double denominator = std::max(1e-12, 1.0 + g2 - (2.0 * g * cosTheta));

		return ((1.0 - g2) / (4.0 * D_PI * denominator * std::sqrt(denominator)));
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
		if (scatterRecord.pdfType == SCATTER_PDF_COSINE)
		{
			const double cosine = Utilities::dot(direction, scatterRecord.cosineBasis.getW());
			if (cosine <= 0.0)
			{
				return (Color(0.0, 0.0, 0.0));
			}
			return (scatterRecord.attenuation * (cosine / D_PI));
		}
		if (scatterRecord.pdfType == SCATTER_PDF_SPHERE)
		{
			return (scatterRecord.attenuation * (1.0 / (4.0 * D_PI)));
		}
		if (scatterRecord.pdfType == SCATTER_PDF_HENYEY_GREENSTEIN)
		{
			return (scatterRecord.attenuation * phasePDF(scatterRecord, direction));
		}
		return (Color(0.0, 0.0, 0.0));
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

	bool	applyRussianRoulette(Color& throughput, int bounces)
	{
		if (bounces < RUSSIAN_ROULETTE_START_BOUNCE)
		{
			return (true);
		}

		double survivalProbability = maxChannel(throughput);
		survivalProbability = std::clamp(
			survivalProbability,
			RUSSIAN_ROULETTE_MIN_SURVIVAL,
			RUSSIAN_ROULETTE_MAX_SURVIVAL
		);
		if (Sampler::sample1D(Sampler::DIM_RUSSIAN_ROULETTE) > survivalProbability)
		{
			return (false);
		}
		throughput /= survivalProbability;
		return (true);
	}

	double	lightSelectionProbability(Scene& scene, std::size_t index)
	{
		const auto& lights = scene.getLights();
		const auto& cumulativeWeights = scene.getLightSelectionCumulativeWeights();

		if (lights.empty())
		{
			return (0.0);
		}
		if (cumulativeWeights.empty() || scene.getLightSelectionTotalWeight() <= 0.0)
		{
			return (1.0 / static_cast<double>(lights.size()));
		}
		const double previousWeight = index == 0 ? 0.0 : cumulativeWeights[index - 1];
		const double weight = cumulativeWeights[index] - previousWeight;
		if (weight <= 0.0)
		{
			return (0.0);
		}
		return (weight / scene.getLightSelectionTotalWeight());
	}

	std::size_t	selectLight(Scene& scene)
	{
		const auto& lights = scene.getLights();
		const auto& cumulativeWeights = scene.getLightSelectionCumulativeWeights();

		if (lights.size() <= 1)
		{
			return (0);
		}
		if (!cumulativeWeights.empty() && scene.getLightSelectionTotalWeight() > 0.0)
		{
			const double target = Sampler::sample1D(Sampler::DIM_LIGHT_SELECTION)
				* scene.getLightSelectionTotalWeight();
			const auto it = std::upper_bound(cumulativeWeights.begin(), cumulativeWeights.end(), target);

			return (std::min<std::size_t>(
				static_cast<std::size_t>(it - cumulativeWeights.begin()),
				lights.size() - 1
			));
		}
		return (std::min<std::size_t>(
			static_cast<std::size_t>(Sampler::sample1D(Sampler::DIM_LIGHT_SELECTION) * lights.size()),
			lights.size() - 1
		));
	}

	bool	materialIsTransport(Material* material)
	{
		if (!material)
		{
			return (false);
		}
		if (material->getType() == METAL || material->getType() == DIELECTRIC)
		{
			return (true);
		}
		if (material->getType() != PRINCIPLED)
		{
			return (false);
		}

		const Principled* principled = dynamic_cast<const Principled*>(material);
		return (
			principled != nullptr
			&& (
				principled->getMetallic() >= 0.5
				|| principled->getTransmission() > 0.05
				|| principled->getClearcoat() > 0.5
			)
		);
	}

	bool	materialReceivesCaustics(Material* material)
	{
		if (!material || material->getType() == EMISSIVE)
		{
			return (false);
		}
		return (!materialIsTransport(material));
	}

	bool	sceneBoundingBox(Scene& scene, AABB& bounds)
	{
		bool hasBounds = false;
		Vector3 minimum;
		Vector3 maximum;

		for (const std::shared_ptr<Hittable>& hittable : scene.getHittables())
		{
			if (!hittable || dynamic_cast<DirectionalLight*>(hittable.get()) != nullptr)
			{
				continue;
			}

			AABB hittableBox;
			if (!hittable->createBoundingBox(hittableBox))
			{
				continue;
			}
			if (!hasBounds)
			{
				minimum = hittableBox.getMinimum();
				maximum = hittableBox.getMaximum();
				hasBounds = true;
				continue;
			}
			minimum = Vector3(
				std::min(minimum.getX(), hittableBox.getMinimum().getX()),
				std::min(minimum.getY(), hittableBox.getMinimum().getY()),
				std::min(minimum.getZ(), hittableBox.getMinimum().getZ())
			);
			maximum = Vector3(
				std::max(maximum.getX(), hittableBox.getMaximum().getX()),
				std::max(maximum.getY(), hittableBox.getMaximum().getY()),
				std::max(maximum.getZ(), hittableBox.getMaximum().getZ())
			);
		}

		if (!hasBounds)
		{
			return (false);
		}
		bounds = AABB(minimum, maximum);
		return (true);
	}

	bool	sampleDirectionalEmission(
		Scene& scene,
		const DirectionalLight& light,
		HittableEmissionSample& sample
	)
	{
		sample = HittableEmissionSample();
		Material* material = light.getMaterial();
		if (!material)
		{
			return (false);
		}

		const Color emitted = material->emitted();
		if (!validPositiveColor(emitted))
		{
			return (false);
		}

		AABB bounds;
		if (!sceneBoundingBox(scene, bounds))
		{
			return (false);
		}

		const Vector3 center = (bounds.getMinimum() + bounds.getMaximum()) * 0.5;
		const Vector3 diagonal = bounds.getMaximum() - bounds.getMinimum();
		double radius = Utilities::vectorLength(diagonal) * 0.5;
		if (!std::isfinite(radius) || radius <= 0.0)
		{
			radius = 1.0;
		}
		radius += std::max(T_MIN, scene.getCausticRadiusMeters() / scene.getMetersPerUnit());

		const Vector3 direction = Utilities::normalize(light.getDirection());
		const ONB basis(direction);
		const Vector3 disk = Sampler::unitDisk(Sampler::DIM_LIGHT_EMISSION_DIRECTION) * radius;

		sample.position = center - direction * (radius + T_MIN) + basis.local(disk.getX(), disk.getY(), 0.0);
		sample.normal = direction * -1.0;
		sample.direction = direction;
		sample.emitted = emitted;
		sample.powerScale = D_PI * radius * radius;
		sample.valid = true;
		return (true);
	}

	bool	samplePhotonEmission(
		Scene& scene,
		HittableEmissionSample& sample,
		double& selectionProbability
	)
	{
		const auto& lights = scene.getLights();

		sample = HittableEmissionSample();
		selectionProbability = 0.0;
		if (lights.empty())
		{
			return (false);
		}

		const std::size_t lightIndex = selectLight(scene);
		selectionProbability = lightSelectionProbability(scene, lightIndex);
		if (selectionProbability <= 0.0 || !std::isfinite(selectionProbability))
		{
			return (false);
		}

		const DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(lights[lightIndex].get());
		if (directionalLight)
		{
			return (sampleDirectionalEmission(scene, *directionalLight, sample));
		}
		return (lights[lightIndex]->sampleEmission(sample));
	}

	double	progressiveRadiusMeters(Scene& scene)
	{
		double radius = scene.getCausticRadiusMeters();
		const int passes = std::max(1, scene.getCausticPassCount());
		const double alpha = scene.getCausticAlpha();

		for (int pass = 1; pass < passes; pass++)
		{
			radius *= std::sqrt((static_cast<double>(pass) + alpha) / (static_cast<double>(pass) + 1.0));
		}
		return (radius);
	}
}

std::size_t	CausticPhotonMap::GridKeyHash::operator()(const GridKey& key) const
{
	const std::uint64_t x = static_cast<std::uint64_t>(key.x) * 0x9e3779b185ebca87ull;
	const std::uint64_t y = static_cast<std::uint64_t>(key.y) * 0xc2b2ae3d27d4eb4full;
	const std::uint64_t z = static_cast<std::uint64_t>(key.z) * 0x165667b19e3779f9ull;

	return (static_cast<std::size_t>(x ^ y ^ z));
}

void	CausticPhotonMap::clear(void)
{
	this->_photons.clear();
	this->_grid.clear();
	this->_radiusMeters = 0.0;
	this->_radiusSceneUnits = 0.0;
	this->_gatherAreaSquareMeters = 0.0;
}

std::size_t	CausticPhotonMap::photonCount(void) const
{
	return (this->_photons.size());
}

double	CausticPhotonMap::radiusMeters(void) const
{
	return (this->_radiusMeters);
}

double	CausticPhotonMap::radiusSceneUnits(void) const
{
	return (this->_radiusSceneUnits);
}

CausticPhotonMap::GridKey	CausticPhotonMap::gridKey(const Vector3& position) const
{
	return (GridKey{
		static_cast<long long>(std::floor(position.getX() / this->_radiusSceneUnits)),
		static_cast<long long>(std::floor(position.getY() / this->_radiusSceneUnits)),
		static_cast<long long>(std::floor(position.getZ() / this->_radiusSceneUnits))
	});
}

void	CausticPhotonMap::storePhoton(const Photon& photon)
{
	this->_photons.push_back(photon);
}

void	CausticPhotonMap::rebuildGrid(void)
{
	this->_grid.clear();
	if (this->_radiusSceneUnits <= 0.0)
	{
		return;
	}
	for (std::size_t i = 0; i < this->_photons.size(); i++)
	{
		this->_grid[this->gridKey(this->_photons[i].position)].push_back(i);
	}
}

void	CausticPhotonMap::build(Scene& scene, std::uint32_t renderSeed)
{
	this->clear();
	if (!scene.getCausticsEnabled() || scene.getCausticPhotonCount() <= 0 || scene.getLights().empty())
	{
		return;
	}

	this->_radiusMeters = progressiveRadiusMeters(scene);
	this->_radiusSceneUnits = this->_radiusMeters / scene.getMetersPerUnit();
	this->_gatherAreaSquareMeters = D_PI * this->_radiusMeters * this->_radiusMeters;
	if (
		this->_radiusMeters <= 0.0
		|| this->_radiusSceneUnits <= 0.0
		|| this->_gatherAreaSquareMeters <= 0.0
		|| !std::isfinite(this->_radiusSceneUnits)
	)
	{
		this->clear();
		return;
	}

	const int photonTarget = scene.getCausticPhotonCount();
	const int passCount = std::max(1, scene.getCausticPassCount());
	const int maxBounces = scene.getMaxLightBounces();
	const double squareMetersPerSceneArea = scene.getMetersPerUnit() * scene.getMetersPerUnit();
	this->_photons.reserve(static_cast<std::size_t>(photonTarget));

	Sampler::setRenderSeed(renderSeed ^ CAUSTIC_SEED_SCRAMBLE);
	for (int pass = 0; pass < passCount; pass++)
	{
		const int passStart = photonTarget * pass / passCount;
		const int passEnd = photonTarget * (pass + 1) / passCount;

		for (int photonIndex = passStart; photonIndex < passEnd; photonIndex++)
		{
			Sampler::beginPixelSample(
				static_cast<std::size_t>(photonIndex),
				static_cast<std::size_t>(pass),
				static_cast<std::uint32_t>(photonIndex - passStart),
				CAUSTIC_STREAM
			);

			HittableEmissionSample emissionSample;
			double selectionProbability = 0.0;
			if (!samplePhotonEmission(scene, emissionSample, selectionProbability) || !emissionSample.valid)
			{
				Sampler::endPixelSample();
				continue;
			}

			Color throughput = emissionSample.emitted
				* (emissionSample.powerScale * squareMetersPerSceneArea)
				/ (static_cast<double>(photonTarget) * selectionProbability);
			if (!validPositiveColor(throughput))
			{
				Sampler::endPixelSample();
				continue;
			}

			Ray currentRay = Ray::fromNormalizedDirection(
				emissionSample.position + emissionSample.direction * (2.0 * T_MIN),
				emissionSample.direction
			);
			bool hasCausticTransport = false;

			for (int bounces = 0; bounces <= maxBounces; bounces++)
			{
				Sampler::setBounce(static_cast<std::uint32_t>(bounces));

				HitRecord hitRecord;
				if (!Renderer::internal::_checkHits(scene, currentRay, hitRecord))
				{
					break;
				}
				if (!hitRecord.material || hitRecord.material->getType() == EMISSIVE)
				{
					break;
				}
				if (hasCausticTransport && materialReceivesCaustics(hitRecord.material))
				{
					this->storePhoton(Photon{
						hitRecord.position,
						hitRecord.normal,
						currentRay.getDirection(),
						throughput
					});
					break;
				}

				ScatterRecord scatterRecord;
				if (!hitRecord.material->scatter(currentRay, hitRecord, scatterRecord))
				{
					break;
				}

				const Color attenuation = effectiveScatterAttenuation(scene, hitRecord, scatterRecord);
				if (!validPositiveColor(attenuation))
				{
					break;
				}

				const bool transportEvent = scatterRecord.isSpecular || materialIsTransport(hitRecord.material);
				if (!transportEvent)
				{
					break;
				}

				throughput = throughput * attenuation;
				if (!validPositiveColor(throughput) || !applyRussianRoulette(throughput, bounces))
				{
					break;
				}

				currentRay = scatterRecord.isSpecular
					? scatterRecord.specularRay
					: Ray::fromNormalizedDirection(hitRecord.position, scatterRecord.sampledDirection);
				hasCausticTransport = true;
			}

			Sampler::endPixelSample();
		}
	}
	Sampler::setRenderSeed(renderSeed);
	this->rebuildGrid();
}

Color	CausticPhotonMap::estimate(const HitRecord& hitRecord, const ScatterRecord& scatterRecord) const
{
	if (
		this->_photons.empty()
		|| this->_radiusSceneUnits <= 0.0
		|| this->_gatherAreaSquareMeters <= 0.0
		|| !hitRecord.material
	)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	const GridKey centerKey = this->gridKey(hitRecord.position);
	const double radiusSquared = this->_radiusSceneUnits * this->_radiusSceneUnits;
	Color result(0.0, 0.0, 0.0);

	for (long long z = -1; z <= 1; z++)
	{
		for (long long y = -1; y <= 1; y++)
		{
			for (long long x = -1; x <= 1; x++)
			{
				const GridKey key{centerKey.x + x, centerKey.y + y, centerKey.z + z};
				const auto cellIt = this->_grid.find(key);
				if (cellIt == this->_grid.end())
				{
					continue;
				}

				for (std::size_t photonIndex : cellIt->second)
				{
					const Photon& photon = this->_photons[photonIndex];
					const Vector3 offset = photon.position - hitRecord.position;
					if (Utilities::vectorLengthSquared(offset) > radiusSquared)
					{
						continue;
					}
					if (Utilities::dot(hitRecord.normal, photon.normal) <= 0.25)
					{
						continue;
					}

					const Vector3 surfaceToLight = photon.incomingDirection * -1.0;
					const double cosine = std::abs(Utilities::dot(hitRecord.normal, surfaceToLight));
					if (cosine <= 1e-6)
					{
						continue;
					}

					const Color bsdfCos = scatterBSDFCos(scatterRecord, hitRecord, surfaceToLight);
					if (!validPositiveColor(bsdfCos))
					{
						continue;
					}
					result += photon.flux * (bsdfCos / cosine);
				}
			}
		}
	}

	return (result / this->_gatherAreaSquareMeters);
}
