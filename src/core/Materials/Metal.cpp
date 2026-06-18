#include "Materials/Metal.hpp"
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

	double	conductorFresnelChannel(double eta, double k, double cosTheta)
	{
		const double cosThetaSquared = cosTheta * cosTheta;
		const double etaSquared = eta * eta;
		const double kSquared = k * k;
		const double etaK = etaSquared + kSquared;
		const double twoEtaCosTheta = 2.0 * eta * cosTheta;
		const double rParallel = (
			(etaK * cosThetaSquared - twoEtaCosTheta + 1.0)
			/ (etaK * cosThetaSquared + twoEtaCosTheta + 1.0)
		);
		const double rPerpendicular = (
			(etaK - twoEtaCosTheta + cosThetaSquared)
			/ (etaK + twoEtaCosTheta + cosThetaSquared)
		);

		return (std::clamp(0.5 * (rParallel + rPerpendicular), 0.0, 1.0));
	}

	Color	conductorFresnel(Color eta, Color extinctionCoefficient, double cosTheta)
	{
		cosTheta = std::clamp(cosTheta, 0.0, 1.0);
		return (Color(
			conductorFresnelChannel(eta.getRed(), extinctionCoefficient.getRed(), cosTheta),
			conductorFresnelChannel(eta.getGreen(), extinctionCoefficient.getGreen(), cosTheta),
			conductorFresnelChannel(eta.getBlue(), extinctionCoefficient.getBlue(), cosTheta)
		));
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
	this->_reflectionFuzziness = reflectionFuzziness;
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
}

Metal::Metal(Color color, double reflectionFuzziness)
{
	this->_color = color;
	this->_reflectionFuzziness = reflectionFuzziness;
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
}

Metal::Metal(Color eta, Color extinctionCoefficient, double reflectionFuzziness)
{
	this->_color = Color(1.0, 1.0, 1.0);
	this->_reflectionFuzziness = reflectionFuzziness;
	this->_useConductorFresnel = false;
	this->_eta = Color(1.0, 1.0, 1.0);
	this->_extinctionCoefficient = Color(0.0, 0.0, 0.0);
	this->setConductorFresnel(eta, extinctionCoefficient);
}

bool	Metal::usesConductorFresnel(void) const
{
	return (this->_useConductorFresnel);
}

Color	Metal::getEta(void) const
{
	return (this->_eta);
}

Color	Metal::getExtinctionCoefficient(void) const
{
	return (this->_extinctionCoefficient);
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
	Vector3	reflected = Utilities::reflect(ray.getDirection(), hitRecord.normal);
	const double cosTheta = std::min(1.0, std::max(0.0, Utilities::dot(ray.getDirection() * -1.0, hitRecord.normal)));

	scatterRecord.specularRay = Ray(hitRecord.position, reflected + this->_reflectionFuzziness * Sampler::unitBall(Sampler::DIM_MATERIAL_FUZZ));
	scatterRecord.attenuation = this->_useConductorFresnel
		? conductorFresnel(this->_eta, this->_extinctionCoefficient, cosTheta)
		: this->colorAt(hitRecord);
	scatterRecord.isSpecular = true;
	scatterRecord.pdfType = SCATTER_PDF_NONE;

	return (true);
}

MaterialType	Metal::getType(void) const
{
	return (METAL);
}
