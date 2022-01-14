#include "Materials/Lambertian.hpp"
#include "PDFs/CosinePDF.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <memory>

Lambertian::Lambertian(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
}

Lambertian::Lambertian(Color color)
{
	this->_color = color;
}

bool	Lambertian::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	scatterRecord.isSpecular = false;
	scatterRecord.attenuation = this->_color;
	scatterRecord.pdfPtr = std::make_shared<CosinePDF>(hitRecord.normal);

	return (true);
	(void)ray;
}

double	Lambertian::scatteringPDF(Ray& ray, HitRecord& hitRecord)
{
	double cosine = Utilities::dot(hitRecord.normal, Utilities::normalize(ray.getDirection()));

	return (cosine < 0.0 ? 0.0 : cosine / D_PI);
}

MaterialType	Lambertian::getType(void) const
{
	return (LAMBERTIAN);
}