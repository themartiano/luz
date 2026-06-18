#include "Materials/Dielectric.hpp"
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
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

Dielectric::Dielectric(Color color)
{
	this->_color = color;
	this->_refractiveIndex = RI_GLASS;
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

Dielectric::Dielectric(Color color, double refractiveIndex)
{
	this->_color = color;
	this->_refractiveIndex = validRefractiveIndex(refractiveIndex);
	this->_absorptionCoefficient = Color(0.0, 0.0, 0.0);
}

double	Dielectric::getRefractiveIndex(void) const
{
	return (this->_refractiveIndex);
}

Color	Dielectric::getAbsorptionCoefficient(void) const
{
	return (this->_absorptionCoefficient);
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
	scatterRecord.isSpecular = true;
	scatterRecord.pdfType = SCATTER_PDF_NONE;
	scatterRecord.attenuation = this->colorAt(hitRecord);
	scatterRecord.hasMediumAbsorption = (
		this->_absorptionCoefficient.getRed() > 0.0
		|| this->_absorptionCoefficient.getGreen() > 0.0
		|| this->_absorptionCoefficient.getBlue() > 0.0
	);
	scatterRecord.mediumAbsorptionCoefficient = this->_absorptionCoefficient;

	double	refractionRatio = this->_refractiveIndex;
	const Vector3&	normalizedDirection = ray.getDirection();
	if (hitRecord.frontFace)
	{
		refractionRatio = 1.0 / refractionRatio;
	}

	double	cosTheta = fmin(Utilities::dot(normalizedDirection * -1.0, hitRecord.normal), 1.0);
	double	sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	bool	cannotRefract = refractionRatio * sinTheta > 1.0;

	Vector3	direction;
	if (cannotRefract || Utilities::schlick(cosTheta, refractionRatio) > Sampler::sample1D(Sampler::DIM_MATERIAL_DECISION))
	{
		direction = Utilities::reflect(normalizedDirection, hitRecord.normal);
	}
	else
	{
		direction = Utilities::refract(normalizedDirection, hitRecord.normal, refractionRatio);
	}

	scatterRecord.specularRay = Ray(hitRecord.position, direction);

	return (true);
}

MaterialType	Dielectric::getType(void) const
{
	return (DIELECTRIC);
}
