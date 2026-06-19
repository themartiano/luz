#include "Materials/DiffuseGlossy.hpp"
#include "Materials/BSDF.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Utilities.hpp"
#include "Sampler.hpp"
#include <cmath>
#include <stdexcept>

namespace
{
	double	validUnit(double value, const std::string& label)
	{
		if (!std::isfinite(value) || value < 0.0 || value > 1.0)
		{
			throw std::invalid_argument(label + " must be finite and in [0,1].");
		}
		return (value);
	}

	Vector3	viewDirection(const Ray& ray, const Vector3& normal)
	{
		return (BSDF::safeNormalize(ray.getDirection() * -1.0, normal));
	}

	double	cosineHemispherePDF(const Vector3& normal, const Vector3& direction)
	{
		const double cosine = Utilities::dot(normal, direction);

		return (cosine <= 0.0 ? 0.0 : cosine / D_PI);
	}

	Color	diffuseBSDFCos(Color color, const Vector3& normal, const Vector3& direction)
	{
		const double noL = Utilities::dot(normal, direction);

		if (noL <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		return (BSDF::clampColor01(color) * (noL / D_PI));
	}

	Vector3	sampleGlossyDirection(const Ray& ray, const Vector3& normal, double roughness)
	{
		const Vector3 view = viewDirection(ray, normal);
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
}

DiffuseGlossy::DiffuseGlossy(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_glossyColor = Color(1.0, 1.0, 1.0);
	this->_glossyWeight = 0.5;
	this->_glossyRoughness = 0.0;
}

DiffuseGlossy::DiffuseGlossy(
	Color diffuseColor,
	Color glossyColor,
	double glossyWeight,
	double glossyRoughness
)
{
	this->_color = diffuseColor;
	this->_glossyColor = glossyColor;
	this->_glossyWeight = validUnit(glossyWeight, "Diffuse-glossy weight");
	this->_glossyRoughness = validUnit(glossyRoughness, "Diffuse-glossy roughness");
}

Color	DiffuseGlossy::getGlossyColor(void) const
{
	return (this->_glossyColor);
}

double	DiffuseGlossy::getGlossyWeight(void) const
{
	return (this->_glossyWeight);
}

double	DiffuseGlossy::getGlossyRoughness(void) const
{
	return (this->_glossyRoughness);
}

void	DiffuseGlossy::setGlossyColor(Color glossyColor)
{
	this->_glossyColor = glossyColor;
}

void	DiffuseGlossy::setGlossyWeight(double glossyWeight)
{
	this->_glossyWeight = validUnit(glossyWeight, "Diffuse-glossy weight");
}

void	DiffuseGlossy::setGlossyRoughness(double glossyRoughness)
{
	this->_glossyRoughness = validUnit(glossyRoughness, "Diffuse-glossy roughness");
}

bool	DiffuseGlossy::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	Vector3 direction;

	if (Sampler::sample1D(Sampler::DIM_MATERIAL_DECISION) < this->_glossyWeight)
	{
		direction = sampleGlossyDirection(ray, hitRecord.normal, this->_glossyRoughness);
	}
	else
	{
		const ONB basis(hitRecord.normal);

		direction = basis.local(Sampler::cosineHemisphere(Sampler::DIM_BSDF_DIRECTION));
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

Color	DiffuseGlossy::evaluateBSDFCos(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Color diffuse = diffuseBSDFCos(this->colorAt(hitRecord), hitRecord.normal, scatteredDirection);
	const Color glossy = BSDF::ggxReflectionBSDFCos(
		BSDF::clampColor01(this->_glossyColor),
		hitRecord.normal,
		viewDirection(ray, hitRecord.normal),
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_glossyRoughness)
	);

	return (diffuse * (1.0 - this->_glossyWeight) + glossy * this->_glossyWeight);
}

double	DiffuseGlossy::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	const Vector3 view = viewDirection(ray, hitRecord.normal);
	const double diffusePDF = cosineHemispherePDF(hitRecord.normal, scatteredDirection);
	const double glossyPDF = BSDF::ggxReflectionPDF(
		hitRecord.normal,
		view,
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_glossyRoughness)
	);

	return (diffusePDF * (1.0 - this->_glossyWeight) + glossyPDF * this->_glossyWeight);
}

MaterialType	DiffuseGlossy::getType(void) const
{
	return (DIFFUSE_GLOSSY);
}
