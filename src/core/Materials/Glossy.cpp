#include "Materials/Glossy.hpp"
#include "Materials/BSDF.hpp"
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

	Vector3	sampleReflectionDirection(const Ray& ray, const Vector3& normal, double roughness)
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

Glossy::Glossy(void)
{
	this->_color = Color(1.0, 1.0, 1.0);
	this->_roughness = 0.0;
}

Glossy::Glossy(Color color, double roughness)
{
	this->_color = color;
	this->_roughness = validUnit(roughness, "Glossy roughness");
}

double	Glossy::getRoughness(void) const
{
	return (this->_roughness);
}

void	Glossy::setRoughness(double roughness)
{
	this->_roughness = validUnit(roughness, "Glossy roughness");
}

bool	Glossy::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	const Vector3 direction = sampleReflectionDirection(ray, hitRecord.normal, this->_roughness);
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

Color	Glossy::evaluateBSDFCos(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	return (BSDF::ggxReflectionBSDFCos(
		BSDF::clampColor01(this->colorAt(hitRecord)),
		hitRecord.normal,
		viewDirection(ray, hitRecord.normal),
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_roughness)
	));
}

double	Glossy::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	return (BSDF::ggxReflectionPDF(
		hitRecord.normal,
		viewDirection(ray, hitRecord.normal),
		scatteredDirection,
		BSDF::roughnessToAlpha(this->_roughness)
	));
}

MaterialType	Glossy::getType(void) const
{
	return (GLOSSY);
}
