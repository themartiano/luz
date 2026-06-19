#include "Materials/Dielectric.hpp"
#include "Materials/BSDF.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "RefractiveIndexes.hpp"
#include "Sampler.hpp"
#include <cmath>
#include <stdexcept>

namespace
{
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

	double	validRefractiveIndex(double refractiveIndex)
	{
		if (!std::isfinite(refractiveIndex) || refractiveIndex <= 0.0)
		{
			throw std::invalid_argument("Dielectric refractive index must be finite and positive.");
		}
		return (refractiveIndex);
	}

	double	validRoughness(double roughness)
	{
		if (!std::isfinite(roughness) || roughness < 0.0 || roughness > 1.0)
		{
			throw std::invalid_argument("Dielectric roughness must be finite and in [0,1].");
		}
		return (roughness);
	}

	double	absorptionFromTransmittance(double transmittance, double distanceMeters)
	{
		if (!std::isfinite(transmittance) || transmittance < 0.0 || transmittance > 1.0)
		{
			throw std::invalid_argument("Dielectric transmittance channels must be finite and in [0,1].");
		}
		if (transmittance <= 0.0)
		{
			return (T_MAX);
		}
		return (-std::log(transmittance) / distanceMeters);
	}
}

Dielectric::Dielectric(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_refractiveIndex = RI_GLASS;
	this->_roughness = 0.0;
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

Dielectric::Dielectric(Color color)
{
	this->_color = color;
	this->_refractiveIndex = RI_GLASS;
	this->_roughness = 0.0;
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

Dielectric::Dielectric(Color color, double refractiveIndex)
{
	this->_color = color;
	this->_refractiveIndex = validRefractiveIndex(refractiveIndex);
	this->_roughness = 0.0;
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

Dielectric::Dielectric(Color color, double refractiveIndex, double roughness)
{
	this->_color = color;
	this->_refractiveIndex = validRefractiveIndex(refractiveIndex);
	this->_roughness = validRoughness(roughness);
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

double	Dielectric::getRefractiveIndex(void) const
{
	return (this->_refractiveIndex);
}

double	Dielectric::getRoughness(void) const
{
	return (this->_roughness);
}

Color	Dielectric::getAbsorptionCoefficient(void) const
{
	return (this->_absorptionCoefficient);
}

void	Dielectric::setRoughness(double roughness)
{
	this->_roughness = validRoughness(roughness);
}

void	Dielectric::setAbsorptionCoefficient(Color absorptionCoefficient)
{
	requireFiniteNonNegativeColor(absorptionCoefficient, "Dielectric absorption coefficient");
	this->_absorptionCoefficient = absorptionCoefficient;
}

void	Dielectric::setTransmittance(Color transmittance, double distanceMeters)
{
	if (!std::isfinite(distanceMeters) || distanceMeters <= 0.0)
	{
		throw std::invalid_argument("Dielectric transmittance distance must be finite and positive.");
	}
	this->_absorptionCoefficient = Color(
		absorptionFromTransmittance(transmittance.getRed(), distanceMeters),
		absorptionFromTransmittance(transmittance.getGreen(), distanceMeters),
		absorptionFromTransmittance(transmittance.getBlue(), distanceMeters)
	);
}

bool	Dielectric::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	scatterRecord.hasMediumAbsorption = (
		this->_absorptionCoefficient.getRed() > 0.0
		|| this->_absorptionCoefficient.getGreen() > 0.0
		|| this->_absorptionCoefficient.getBlue() > 0.0
	);
	scatterRecord.mediumAbsorptionCoefficient = this->_absorptionCoefficient;

	const Vector3& normalizedDirection = ray.getDirection();
	const Vector3 view = BSDF::safeNormalize(normalizedDirection * -1.0, hitRecord.normal);
	const double etaI = hitRecord.frontFace ? 1.0 : this->_refractiveIndex;
	const double etaT = hitRecord.frontFace ? this->_refractiveIndex : 1.0;
	const double refractionRatio = etaI / etaT;
	const double alpha = BSDF::roughnessToAlpha(this->_roughness);

	if (this->_roughness <= 1e-4)
	{
		double cosTheta = fmin(Utilities::dot(view, hitRecord.normal), 1.0);
		double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
		bool cannotRefract = refractionRatio * sinTheta > 1.0;
		Vector3 direction;

		if (
			cannotRefract
			|| BSDF::dielectricFresnel(cosTheta, etaI, etaT) > Sampler::sample1D(Sampler::DIM_MATERIAL_DECISION)
		)
		{
			direction = Utilities::reflect(normalizedDirection, hitRecord.normal);
			scatterRecord.attenuation = Color(1.0, 1.0, 1.0);
		}
		else
		{
			direction = Utilities::refract(normalizedDirection, hitRecord.normal, refractionRatio);
			scatterRecord.attenuation = this->colorAt(hitRecord);
		}

		scatterRecord.isSpecular = true;
		scatterRecord.pdfType = SCATTER_PDF_NONE;
		scatterRecord.specularRay = Ray(hitRecord.position, direction);
		return (true);
	}

	Vector3 halfVector = BSDF::sampleGGXHalfVector(
		hitRecord.normal,
		alpha,
		Sampler::sample2D(Sampler::DIM_BSDF_DIRECTION)
	);
	if (Utilities::dot(view, halfVector) <= 0.0)
	{
		halfVector = hitRecord.normal;
	}

	const double fresnel = BSDF::dielectricFresnel(Utilities::dot(view, halfVector), etaI, etaT);
	Vector3 refractedDirection;
	const bool canRefract = BSDF::refractDirection(normalizedDirection, halfVector, refractionRatio, refractedDirection);
	Vector3 direction;
	if (!canRefract || Sampler::sample1D(Sampler::DIM_MATERIAL_DECISION) < fresnel)
	{
		direction = Utilities::reflect(normalizedDirection, halfVector);
	}
	else
	{
		direction = refractedDirection;
	}

	const double pdf = this->scatteringPDF(ray, hitRecord, direction);
	const Color bsdfCos = this->evaluateBSDFCos(ray, hitRecord, direction);
	if (pdf <= 0.0 || !std::isfinite(pdf) || BSDF::maxChannel(bsdfCos) <= 0.0)
	{
		return (false);
	}

	scatterRecord.incidentRay = ray;
	scatterRecord.sampledDirection = direction;
	scatterRecord.sampledPDF = pdf;
	scatterRecord.attenuation = bsdfCos / pdf;
	scatterRecord.isSpecular = false;
	scatterRecord.pdfType = SCATTER_PDF_BSDF;
	scatterRecord.bsdfMaterial = this;

	return (true);
}

Color	Dielectric::evaluateBSDFCos(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Vector3 view = BSDF::safeNormalize(ray.getDirection() * -1.0, hitRecord.normal);
	const double etaI = hitRecord.frontFace ? 1.0 : this->_refractiveIndex;
	const double etaT = hitRecord.frontFace ? this->_refractiveIndex : 1.0;

	return (BSDF::ggxDielectricBSDFCos(
		this->colorAt(hitRecord),
		hitRecord.normal,
		view,
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_roughness),
		etaI,
		etaT
	));
}

double	Dielectric::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Vector3 view = BSDF::safeNormalize(ray.getDirection() * -1.0, hitRecord.normal);
	const double etaI = hitRecord.frontFace ? 1.0 : this->_refractiveIndex;
	const double etaT = hitRecord.frontFace ? this->_refractiveIndex : 1.0;

	return (BSDF::ggxDielectricPDF(
		hitRecord.normal,
		view,
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_roughness),
		etaI,
		etaT
	));
}

MaterialType	Dielectric::getType(void) const
{
	return (DIELECTRIC);
}
