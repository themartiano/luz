#include "Materials/HenyeyGreenstein.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <stdexcept>

namespace
{
	constexpr double	MAX_ABS_ANISOTROPY = 0.99;

	double	sanitizedAnisotropy(double anisotropy)
	{
		if (!std::isfinite(anisotropy) || anisotropy < -MAX_ABS_ANISOTROPY || anisotropy > MAX_ABS_ANISOTROPY)
		{
			throw std::invalid_argument("Volume anisotropy must be finite and between -0.99 and 0.99.");
		}
		return (anisotropy);
	}

	Vector3	phaseDirectionOrFallback(const Ray& ray)
	{
		const Vector3 direction = ray.getDirection();

		if (Utilities::vectorLengthSquared(direction) <= 0.0)
		{
			return (Vector3(0.0, 0.0, 1.0));
		}
		return (Utilities::normalize(direction));
	}
}

HenyeyGreenstein::HenyeyGreenstein(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_anisotropy = 0.0;
}

HenyeyGreenstein::HenyeyGreenstein(Color color, double anisotropy)
{
	this->_color = color;
	this->_anisotropy = sanitizedAnisotropy(anisotropy);
}

void	HenyeyGreenstein::setAnisotropy(double anisotropy)
{
	this->_anisotropy = sanitizedAnisotropy(anisotropy);
}

double	HenyeyGreenstein::getAnisotropy(void) const
{
	return (this->_anisotropy);
}

bool	HenyeyGreenstein::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	scatterRecord.isSpecular = false;
	scatterRecord.attenuation = this->colorAt(hitRecord);
	scatterRecord.pdfType = SCATTER_PDF_HENYEY_GREENSTEIN;
	scatterRecord.phaseDirection = phaseDirectionOrFallback(ray);
	scatterRecord.phaseAnisotropy = this->_anisotropy;

	return (true);
}

double	HenyeyGreenstein::scatteringPDF(Ray& ray, HitRecord& hitRecord)
{
	(void)ray;
	(void)hitRecord;
	return (1.0 / (4.0 * D_PI));
}

MaterialType	HenyeyGreenstein::getType(void) const
{
	return (HENYEY_GREENSTEIN);
}
