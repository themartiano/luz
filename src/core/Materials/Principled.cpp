#include "Materials/Principled.hpp"
#include "Defaults.hpp"
#include "Sampler.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cmath>

namespace
{
	double	clamp01(double value)
	{
		return (std::max(0.0, std::min(1.0, value)));
	}

	double	maxChannel(const Color& color)
	{
		return (std::max(color.getRed(), std::max(color.getGreen(), color.getBlue())));
	}

	Color	mixColor(const Color& a, const Color& b, double factor)
	{
		return (a * (1.0 - factor) + b * factor);
	}
}

Principled::Principled(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_metallic = 0.0;
	this->_roughness = 0.5;
}

Principled::Principled(Color color, double metallic, double roughness)
{
	this->_color = color;
	this->_metallic = clamp01(metallic);
	this->_roughness = clamp01(roughness);
}

bool	Principled::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	const Color	baseColor = this->colorAt(hitRecord);
	const Vector3&	normalizedDirection = ray.getDirection();
	const double	cosTheta = std::max(0.0, Utilities::dot(normalizedDirection * -1.0, hitRecord.normal));
	const double	dielectricF0 = 0.04;
	const double	metalF0 = clamp01(maxChannel(baseColor));
	const double	f0 = dielectricF0 * (1.0 - this->_metallic) + metalF0 * this->_metallic;
	double	specularChance = f0 + (1.0 - f0) * std::pow(1.0 - cosTheta, 5.0);

	specularChance += (1.0 - this->_roughness) * (0.25 + 0.25 * this->_metallic);
	specularChance = std::max(0.04, std::min(0.95, specularChance));

	if (Sampler::sample1D(Sampler::DIM_MATERIAL_DECISION) < specularChance)
	{
		const double	fuzz = this->_roughness * this->_roughness;
		const Vector3	reflected = Utilities::reflect(normalizedDirection, hitRecord.normal);
		const Color	specularColor = mixColor(Color(1.0, 1.0, 1.0), baseColor, this->_metallic);

		scatterRecord.specularRay = Ray(hitRecord.position, reflected + fuzz * Sampler::unitBall(Sampler::DIM_MATERIAL_FUZZ));
		scatterRecord.attenuation = specularColor;
		scatterRecord.isSpecular = true;
		scatterRecord.pdfType = SCATTER_PDF_NONE;
		return (true);
	}

	scatterRecord.isSpecular = false;
	scatterRecord.attenuation = baseColor * (1.0 - this->_metallic);
	scatterRecord.pdfType = SCATTER_PDF_COSINE;
	scatterRecord.cosineBasis = ONB(hitRecord.normal);
	return (true);
}

double	Principled::scatteringPDF(Ray& ray, HitRecord& hitRecord)
{
	double cosine = Utilities::dot(hitRecord.normal, ray.getDirection());

	return (cosine < 0.0 ? 0.0 : cosine / D_PI);
}

MaterialType	Principled::getType(void) const
{
	return (PRINCIPLED);
}
