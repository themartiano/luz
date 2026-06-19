#include "Materials/Lambertian.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"

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
	scatterRecord.attenuation = this->colorAt(hitRecord);
	scatterRecord.pdfType = SCATTER_PDF_COSINE;
	scatterRecord.cosineBasis = ONB(hitRecord.normal);

	return (true);
	(void)ray;
}

double	Lambertian::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	(void)ray;
	double cosine = Utilities::dot(hitRecord.normal, scatteredDirection);

	return (cosine < 0.0 ? 0.0 : cosine / D_PI);
}

MaterialType	Lambertian::getType(void) const
{
	return (LAMBERTIAN);
}
