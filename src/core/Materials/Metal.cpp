#include "Materials/Metal.hpp"
#include "Materials/BSDF.hpp"
#include "Utilities.hpp"
#include "Vector3.hpp"
#include "Sampler.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
	void	requirePositiveEta(Color eta)
	{
		if (
			!std::isfinite(eta.getRed()) || eta.getRed() <= 0.0
			|| !std::isfinite(eta.getGreen()) || eta.getGreen() <= 0.0
			|| !std::isfinite(eta.getBlue()) || eta.getBlue() <= 0.0
		)
		{
			throw std::invalid_argument("Conductor eta must have finite positive channels.");
		}
	}

	void	requireNonNegativeExtinction(Color extinctionCoefficient)
	{
		if (
			!std::isfinite(extinctionCoefficient.getRed()) || extinctionCoefficient.getRed() < 0.0
			|| !std::isfinite(extinctionCoefficient.getGreen()) || extinctionCoefficient.getGreen() < 0.0
			|| !std::isfinite(extinctionCoefficient.getBlue()) || extinctionCoefficient.getBlue() < 0.0
		)
		{
			throw std::invalid_argument("Conductor extinction coefficient must have finite non-negative channels.");
		}
	}

	double	validRoughness(double roughness)
	{
		if (!std::isfinite(roughness) || roughness < 0.0 || roughness > 1.0)
		{
			throw std::invalid_argument("Metal roughness must be finite and in [0,1].");
		}
		return (roughness);
	}
}

Metal::Metal(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_reflectionFuzziness = 0.0;
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
}

Metal::Metal(Color color)
{
	this->_color = color;
	this->_reflectionFuzziness = 0.0;
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
}

Metal::Metal(double reflectionFuzziness)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_reflectionFuzziness = validRoughness(reflectionFuzziness);
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
}

Metal::Metal(Color color, double reflectionFuzziness)
{
	this->_color = color;
	this->_reflectionFuzziness = validRoughness(reflectionFuzziness);
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
}

Metal::Metal(Color eta, Color extinctionCoefficient, double reflectionFuzziness)
{
	this->_color = Color(1.0, 1.0, 1.0);
	this->_reflectionFuzziness = validRoughness(reflectionFuzziness);
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
	this->setConductorFresnel(eta, extinctionCoefficient);
}

bool	Metal::usesConductorFresnel(void) const
{
	return (this->_useConductorFresnel);
}

double	Metal::getRoughness(void) const
{
	return (this->_reflectionFuzziness);
}

Color	Metal::getEta(void) const
{
	return (this->_eta);
}

Color	Metal::getExtinctionCoefficient(void) const
{
	return (this->_extinctionCoefficient);
}

void	Metal::setRoughness(double roughness)
{
	this->_reflectionFuzziness = validRoughness(roughness);
}

void	Metal::setConductorFresnel(Color eta, Color extinctionCoefficient)
{
	requirePositiveEta(eta);
	requireNonNegativeExtinction(extinctionCoefficient);
	this->_eta = eta;
	this->_extinctionCoefficient = extinctionCoefficient;
	this->_useConductorFresnel = true;
}

bool	Metal::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	const Vector3 view = BSDF::safeNormalize(ray.getDirection() * -1.0, hitRecord.normal);
	const double cosTheta = std::min(1.0, std::max(0.0, Utilities::dot(view, hitRecord.normal)));
	const double alpha = BSDF::roughnessToAlpha(this->_reflectionFuzziness);

	if (this->_reflectionFuzziness <= 1e-4)
	{
		const Vector3 reflected = Utilities::reflect(ray.getDirection(), hitRecord.normal);

		scatterRecord.specularRay = Ray(hitRecord.position, reflected);
		scatterRecord.attenuation = this->_useConductorFresnel
			? BSDF::conductorFresnel(this->_eta, this->_extinctionCoefficient, cosTheta)
			: BSDF::schlickFresnel(BSDF::clampColor01(this->colorAt(hitRecord)), cosTheta);
		scatterRecord.isSpecular = true;
		scatterRecord.pdfType = SCATTER_PDF_NONE;
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
	Vector3 direction = Utilities::reflect(ray.getDirection(), halfVector);
	if (Utilities::dot(direction, hitRecord.normal) <= 0.0)
	{
		direction = Utilities::reflect(ray.getDirection(), hitRecord.normal);
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

Color	Metal::evaluateBSDFCos(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Vector3 view = BSDF::safeNormalize(ray.getDirection() * -1.0, hitRecord.normal);
	const double alpha = BSDF::roughnessToAlpha(this->_reflectionFuzziness);

	if (this->_useConductorFresnel)
	{
		return (BSDF::ggxConductorBSDFCos(
			this->_eta,
			this->_extinctionCoefficient,
			hitRecord.normal,
			view,
			scatteredDirection,
			alpha
		));
	}
	return (BSDF::ggxSchlickReflectionBSDFCos(
		BSDF::clampColor01(this->colorAt(hitRecord)),
		hitRecord.normal,
		view,
		scatteredDirection,
		alpha
	));
}

double	Metal::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Vector3 view = BSDF::safeNormalize(ray.getDirection() * -1.0, hitRecord.normal);

	return (BSDF::ggxReflectionPDF(
		hitRecord.normal,
		view,
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_reflectionFuzziness)
	));
}

MaterialType	Metal::getType(void) const
{
	return (METAL);
}
