#include "Materials/Principled.hpp"
#include "Materials/BSDF.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "RefractiveIndexes.hpp"
#include "Sampler.hpp"
#include "Utilities.hpp"
#include <array>
#include <cmath>
#include <stdexcept>

namespace
{
	enum Component
	{
		COMPONENT_DIFFUSE,
		COMPONENT_SHEEN,
		COMPONENT_DIELECTRIC_SPECULAR,
		COMPONENT_METAL,
		COMPONENT_TRANSMISSION,
		COMPONENT_CLEARCOAT,
		COMPONENT_COUNT
	};

	struct ComponentWeights
	{
		std::array<double, COMPONENT_COUNT> weights = {};
		double total = 0.0;
	};

	double	validUnit(double value, const std::string& label)
	{
		if (!std::isfinite(value) || value < 0.0 || value > 1.0)
		{
			throw std::invalid_argument(label + " must be finite and in [0,1].");
		}
		return (value);
	}

	double	validPositive(double value, const std::string& label)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			throw std::invalid_argument(label + " must be finite and positive.");
		}
		return (value);
	}

	void	requireFiniteNonNegativeColor(const Color& color, const std::string& label)
	{
		if (
			!std::isfinite(color.getRed()) || color.getRed() < 0.0
			|| !std::isfinite(color.getGreen()) || color.getGreen() < 0.0
			|| !std::isfinite(color.getBlue()) || color.getBlue() < 0.0
		)
		{
			throw std::invalid_argument(label + " must have finite non-negative channels.");
		}
	}

	double	absorptionFromTransmittance(double transmittance, double distanceMeters)
	{
		if (!std::isfinite(transmittance) || transmittance < 0.0 || transmittance > 1.0)
		{
			throw std::invalid_argument("Principled transmittance channels must be finite and in [0,1].");
		}
		if (transmittance <= 0.0)
		{
			return (T_MAX);
		}
		return (-std::log(transmittance) / distanceMeters);
	}

	double	luminanceWeight(Color color)
	{
		const double luminance = Utilities::luminance(color);

		if (!std::isfinite(luminance) || luminance <= 0.0)
		{
			return (0.0);
		}
		return (luminance);
	}

	double	dielectricF0(double refractiveIndex)
	{
		const double r0 = (refractiveIndex - 1.0) / (refractiveIndex + 1.0);

		return (BSDF::clamp01(r0 * r0));
	}

	Vector3	viewDirection(const Ray& ray, const HitRecord& hitRecord)
	{
		return (BSDF::safeNormalize(ray.getDirection() * -1.0, hitRecord.normal));
	}

	double	diffuseScale(
		double metallic,
		double transmission,
		double refractiveIndex,
		double clearcoat
	)
	{
		const double clearcoatEnergy = 0.25 * clearcoat;

		return (
			(1.0 - metallic)
			* (1.0 - transmission)
			* (1.0 - dielectricF0(refractiveIndex))
			* (1.0 - clearcoatEnergy)
		);
	}

	ComponentWeights	componentWeights(
		Color baseColor,
		double metallic,
		double roughness,
		double transmission,
		double refractiveIndex,
		double clearcoat,
		double sheen
	)
	{
		ComponentWeights result;
		const double baseLuminance = std::max(0.05, luminanceWeight(baseColor));
		const double diffuse = diffuseScale(metallic, transmission, refractiveIndex, clearcoat);
		const double dielectricSpecular = (1.0 - metallic) * std::max(0.03, dielectricF0(refractiveIndex));

		result.weights[COMPONENT_DIFFUSE] = diffuse * baseLuminance;
		result.weights[COMPONENT_SHEEN] = diffuse * sheen * 0.35;
		result.weights[COMPONENT_DIELECTRIC_SPECULAR] = dielectricSpecular * (1.0 - 0.5 * transmission);
		result.weights[COMPONENT_METAL] = metallic * std::max(0.05, baseLuminance);
		result.weights[COMPONENT_TRANSMISSION] = (1.0 - metallic) * transmission;
		result.weights[COMPONENT_CLEARCOAT] = clearcoat * 0.25 * (1.0 - 0.5 * roughness);

		for (double weight : result.weights)
		{
			if (std::isfinite(weight) && weight > 0.0)
			{
				result.total += weight;
			}
		}
		if (result.total <= 0.0)
		{
			result.weights[COMPONENT_DIFFUSE] = 1.0;
			result.total = 1.0;
		}
		return (result);
	}

	Component	selectComponent(const ComponentWeights& weights)
	{
		double target = Sampler::sample1D(Sampler::DIM_MATERIAL_DECISION) * weights.total;
		for (int i = 0; i < COMPONENT_COUNT; i++)
		{
			target -= weights.weights[i];
			if (target <= 0.0)
			{
				return (static_cast<Component>(i));
			}
		}
		return (COMPONENT_DIFFUSE);
	}

	double	cosineHemispherePDF(const Vector3& normal, const Vector3& direction)
	{
		const double cosine = Utilities::dot(normal, direction);

		return (cosine <= 0.0 ? 0.0 : cosine / D_PI);
	}

	Color	diffuseBSDFCos(Color baseColor, const Vector3& normal, const Vector3& direction, double scale)
	{
		const double noL = Utilities::dot(normal, direction);

		if (noL <= 0.0 || scale <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		return (baseColor * (scale * noL / D_PI));
	}

	Color	sheenBSDFCos(
		Color baseColor,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& direction,
		double scale
	)
	{
		const double noL = Utilities::dot(normal, direction);

		if (noL <= 0.0 || scale <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		const Vector3 halfVector = BSDF::safeNormalize(view + direction, normal);
		const double sheenFalloff = std::pow(1.0 - BSDF::clamp01(Utilities::dot(direction, halfVector)), 5.0);
		const Color sheenColor = BSDF::mixColor(Color(1.0, 1.0, 1.0), baseColor, 0.5);

		return (sheenColor * (scale * sheenFalloff * noL / D_PI));
	}

	Color	scaled(Color color, double factor)
	{
		if (!std::isfinite(factor) || factor <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		return (color * factor);
	}

	Color	dielectricReflectionBSDFCos(
		double refractiveIndex,
		double roughness,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& direction
	)
	{
		if (Utilities::dot(normal, direction) <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}

		const Vector3 halfVector = BSDF::safeNormalize(view + direction, normal);
		const double fresnel = BSDF::dielectricFresnel(
			Utilities::dot(view, halfVector),
			1.0,
			refractiveIndex
		);

		return (BSDF::ggxReflectionBSDFCos(
			Color(fresnel, fresnel, fresnel),
			normal,
			view,
			direction,
			BSDF::roughnessToAlpha(roughness)
		));
	}

	double	reflectionPDF(
		double roughness,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& direction
	)
	{
		return (BSDF::ggxReflectionPDF(
			normal,
			view,
			direction,
			BSDF::roughnessToAlpha(roughness)
		));
	}

	Vector3	sampleGGXReflection(
		const Ray& ray,
		const Vector3& normal,
		double roughness
	)
	{
		const Vector3 view = BSDF::safeNormalize(ray.getDirection() * -1.0, normal);
		Vector3 halfVector = BSDF::sampleGGXHalfVector(
			normal,
			BSDF::roughnessToAlpha(roughness),
			Sampler::sample2D(Sampler::DIM_BSDF_DIRECTION)
		);

		if (Utilities::dot(view, halfVector) <= 0.0)
		{
			halfVector = normal;
		}
		Vector3 direction = Utilities::reflect(ray.getDirection(), halfVector);
		if (Utilities::dot(direction, normal) <= 0.0)
		{
			direction = Utilities::reflect(ray.getDirection(), normal);
		}
		return (direction);
	}

	Vector3	sampleGGXTransmission(
		const Ray& ray,
		const HitRecord& hitRecord,
		double roughness,
		double refractiveIndex
	)
	{
		const Vector3 view = viewDirection(ray, hitRecord);
		const double etaI = hitRecord.frontFace ? 1.0 : refractiveIndex;
		const double etaT = hitRecord.frontFace ? refractiveIndex : 1.0;
		const double eta = etaI / etaT;
		Vector3 halfVector = BSDF::sampleGGXHalfVector(
			hitRecord.normal,
			BSDF::roughnessToAlpha(roughness),
			Sampler::sample2D(Sampler::DIM_BSDF_DIRECTION)
		);

		if (Utilities::dot(view, halfVector) <= 0.0)
		{
			halfVector = hitRecord.normal;
		}

		Vector3 refracted;
		if (!BSDF::refractDirection(ray.getDirection(), halfVector, eta, refracted))
		{
			return (Utilities::reflect(ray.getDirection(), halfVector));
		}
		return (refracted);
	}
}

Principled::Principled(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_metallic = 0.0;
	this->_roughness = 0.5;
	this->_transmission = 0.0;
	this->_refractiveIndex = RI_GLASS;
	this->_clearcoat = 0.0;
	this->_clearcoatRoughness = 0.03;
	this->_sheen = 0.0;
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

Principled::Principled(Color color, double metallic, double roughness)
	: Principled(color, metallic, roughness, 0.0, RI_GLASS, 0.0, 0.03, 0.0)
{
}

Principled::Principled(
	Color color,
	double metallic,
	double roughness,
	double transmission,
	double refractiveIndex,
	double clearcoat,
	double clearcoatRoughness,
	double sheen
)
{
	this->_color = color;
	this->_metallic = validUnit(metallic, "Principled metallic");
	this->_roughness = validUnit(roughness, "Principled roughness");
	this->_transmission = validUnit(transmission, "Principled transmission");
	this->_refractiveIndex = validPositive(refractiveIndex, "Principled refractive index");
	this->_clearcoat = validUnit(clearcoat, "Principled clearcoat");
	this->_clearcoatRoughness = validUnit(clearcoatRoughness, "Principled clearcoat roughness");
	this->_sheen = validUnit(sheen, "Principled sheen");
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

double	Principled::getMetallic(void) const
{
	return (this->_metallic);
}

double	Principled::getRoughness(void) const
{
	return (this->_roughness);
}

double	Principled::getTransmission(void) const
{
	return (this->_transmission);
}

double	Principled::getRefractiveIndex(void) const
{
	return (this->_refractiveIndex);
}

double	Principled::getClearcoat(void) const
{
	return (this->_clearcoat);
}

double	Principled::getClearcoatRoughness(void) const
{
	return (this->_clearcoatRoughness);
}

double	Principled::getSheen(void) const
{
	return (this->_sheen);
}

Color	Principled::getAbsorptionCoefficient(void) const
{
	return (this->_absorptionCoefficient);
}

void	Principled::setMetallic(double metallic)
{
	this->_metallic = validUnit(metallic, "Principled metallic");
}

void	Principled::setRoughness(double roughness)
{
	this->_roughness = validUnit(roughness, "Principled roughness");
}

void	Principled::setTransmission(double transmission)
{
	this->_transmission = validUnit(transmission, "Principled transmission");
}

void	Principled::setRefractiveIndex(double refractiveIndex)
{
	this->_refractiveIndex = validPositive(refractiveIndex, "Principled refractive index");
}

void	Principled::setClearcoat(double clearcoat)
{
	this->_clearcoat = validUnit(clearcoat, "Principled clearcoat");
}

void	Principled::setClearcoatRoughness(double clearcoatRoughness)
{
	this->_clearcoatRoughness = validUnit(clearcoatRoughness, "Principled clearcoat roughness");
}

void	Principled::setSheen(double sheen)
{
	this->_sheen = validUnit(sheen, "Principled sheen");
}

void	Principled::setAbsorptionCoefficient(Color absorptionCoefficient)
{
	requireFiniteNonNegativeColor(absorptionCoefficient, "Principled absorption coefficient");
	this->_absorptionCoefficient = absorptionCoefficient;
}

void	Principled::setTransmittance(Color transmittance, double distanceMeters)
{
	if (!std::isfinite(distanceMeters) || distanceMeters <= 0.0)
	{
		throw std::invalid_argument("Principled transmittance distance must be finite and positive.");
	}
	this->_absorptionCoefficient = Color(
		absorptionFromTransmittance(transmittance.getRed(), distanceMeters),
		absorptionFromTransmittance(transmittance.getGreen(), distanceMeters),
		absorptionFromTransmittance(transmittance.getBlue(), distanceMeters)
	);
}

bool	Principled::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	const Color baseColor = BSDF::clampColor01(this->colorAt(hitRecord));
	const ComponentWeights weights = componentWeights(
		baseColor,
		this->_metallic,
		this->_roughness,
		this->_transmission,
		this->_refractiveIndex,
		this->_clearcoat,
		this->_sheen
	);
	const Component component = selectComponent(weights);
	Vector3 direction;

	if (component == COMPONENT_DIFFUSE || component == COMPONENT_SHEEN)
	{
		const ONB basis(hitRecord.normal);

		direction = basis.local(Sampler::cosineHemisphere(Sampler::DIM_BSDF_DIRECTION));
	}
	else if (component == COMPONENT_TRANSMISSION)
	{
		direction = sampleGGXTransmission(ray, hitRecord, this->_roughness, this->_refractiveIndex);
	}
	else if (component == COMPONENT_CLEARCOAT)
	{
		direction = sampleGGXReflection(ray, hitRecord.normal, this->_clearcoatRoughness);
	}
	else
	{
		direction = sampleGGXReflection(ray, hitRecord.normal, this->_roughness);
	}

	const double pdf = this->scatteringPDF(ray, hitRecord, direction);
	const Color bsdfCos = this->evaluateBSDFCos(ray, hitRecord, direction);
	if (pdf <= 0.0 || !std::isfinite(pdf) || BSDF::maxChannel(bsdfCos) <= 0.0)
	{
		return (false);
	}

	scatterRecord.hasMediumAbsorption = (
		this->_transmission > 0.0
		&& (
			this->_absorptionCoefficient.getRed() > 0.0
			|| this->_absorptionCoefficient.getGreen() > 0.0
			|| this->_absorptionCoefficient.getBlue() > 0.0
		)
	);
	scatterRecord.mediumAbsorptionCoefficient = this->_absorptionCoefficient;
	scatterRecord.incidentRay = ray;
	scatterRecord.sampledDirection = direction;
	scatterRecord.sampledPDF = pdf;
	scatterRecord.attenuation = bsdfCos / pdf;
	scatterRecord.isSpecular = false;
	scatterRecord.pdfType = SCATTER_PDF_BSDF;
	scatterRecord.bsdfMaterial = this;
	return (true);
}

Color	Principled::evaluateBSDFCos(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Color baseColor = BSDF::clampColor01(this->colorAt(hitRecord));
	const Vector3 view = viewDirection(ray, hitRecord);
	const double alpha = BSDF::roughnessToAlpha(this->_roughness);
	const double clearcoatAlpha = BSDF::roughnessToAlpha(this->_clearcoatRoughness);
	const double transmission = this->_transmission * (1.0 - this->_metallic);
	const double diffuse = diffuseScale(
		this->_metallic,
		this->_transmission,
		this->_refractiveIndex,
		this->_clearcoat
	);
	Color result = diffuseBSDFCos(baseColor, hitRecord.normal, scatteredDirection, diffuse);

	result += sheenBSDFCos(
		baseColor,
		hitRecord.normal,
		view,
		scatteredDirection,
		diffuse * this->_sheen
	);
	result += scaled(
		dielectricReflectionBSDFCos(
			this->_refractiveIndex,
			this->_roughness,
			hitRecord.normal,
			view,
			scatteredDirection
		),
		1.0 - this->_metallic
	);
	result += scaled(
		BSDF::ggxSchlickReflectionBSDFCos(
			baseColor,
			hitRecord.normal,
			view,
			scatteredDirection,
			alpha
		),
		this->_metallic
	);
	if (Utilities::dot(hitRecord.normal, scatteredDirection) < 0.0 && transmission > 0.0)
	{
		const double etaI = hitRecord.frontFace ? 1.0 : this->_refractiveIndex;
		const double etaT = hitRecord.frontFace ? this->_refractiveIndex : 1.0;

		result += scaled(
			BSDF::ggxDielectricBSDFCos(
				baseColor,
				hitRecord.normal,
				view,
				scatteredDirection,
				alpha,
				etaI,
				etaT
			),
			transmission
		);
	}
	result += scaled(
		BSDF::ggxSchlickReflectionBSDFCos(
			Color(0.04, 0.04, 0.04),
			hitRecord.normal,
			view,
			scatteredDirection,
			clearcoatAlpha
		),
		this->_clearcoat * 0.25
	);

	return (result);
}

double	Principled::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Color baseColor = BSDF::clampColor01(this->colorAt(hitRecord));
	const ComponentWeights weights = componentWeights(
		baseColor,
		this->_metallic,
		this->_roughness,
		this->_transmission,
		this->_refractiveIndex,
		this->_clearcoat,
		this->_sheen
	);
	const Vector3 view = viewDirection(ray, hitRecord);
	const double alpha = BSDF::roughnessToAlpha(this->_roughness);
	const double clearcoatAlpha = BSDF::roughnessToAlpha(this->_clearcoatRoughness);
	const double etaI = hitRecord.frontFace ? 1.0 : this->_refractiveIndex;
	const double etaT = hitRecord.frontFace ? this->_refractiveIndex : 1.0;
	std::array<double, COMPONENT_COUNT> pdfs = {};

	pdfs[COMPONENT_DIFFUSE] = cosineHemispherePDF(hitRecord.normal, scatteredDirection);
	pdfs[COMPONENT_SHEEN] = pdfs[COMPONENT_DIFFUSE];
	pdfs[COMPONENT_DIELECTRIC_SPECULAR] = reflectionPDF(
		this->_roughness,
		hitRecord.normal,
		view,
		scatteredDirection
	);
	pdfs[COMPONENT_METAL] = pdfs[COMPONENT_DIELECTRIC_SPECULAR];
	if (Utilities::dot(hitRecord.normal, scatteredDirection) < 0.0)
	{
		pdfs[COMPONENT_TRANSMISSION] = BSDF::ggxDielectricPDF(
			hitRecord.normal,
			view,
			scatteredDirection,
			alpha,
			etaI,
			etaT
		);
	}
	pdfs[COMPONENT_CLEARCOAT] = BSDF::ggxReflectionPDF(
		hitRecord.normal,
		view,
		scatteredDirection,
		clearcoatAlpha
	);

	double pdf = 0.0;
	for (int i = 0; i < COMPONENT_COUNT; i++)
	{
		if (weights.weights[i] > 0.0 && pdfs[i] > 0.0)
		{
			pdf += (weights.weights[i] / weights.total) * pdfs[i];
		}
	}
	return ((std::isfinite(pdf) && pdf > 0.0) ? pdf : 0.0);
}

MaterialType	Principled::getType(void) const
{
	return (PRINCIPLED);
}
