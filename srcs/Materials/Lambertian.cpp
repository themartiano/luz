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

bool	Lambertian::scatter(Ray& ray)
{
	ray.scatterRecord.isSpecular = false;
	ray.scatterRecord.attenuation = this->_color;
	ray.scatterRecord.pdfPtr = std::make_shared<CosinePDF>(ray.hitRecord.normal);

	return (true);
}

double	Lambertian::scatteringPDF(Ray& ray)
{
	double cosine = Utilities::dot(ray.hitRecord.normal, Utilities::normalize(ray.getDirection()));

	return (cosine < 0.0 ? 0.0 : cosine / D_PI);
}