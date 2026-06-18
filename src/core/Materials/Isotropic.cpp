#include "Materials/Isotropic.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"

Isotropic::Isotropic(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
}

Isotropic::Isotropic(Color color)
{
	this->_color = color;
}

bool	Isotropic::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	scatterRecord.isSpecular = false;
	scatterRecord.attenuation = this->colorAt(hitRecord);
	scatterRecord.pdfType = SCATTER_PDF_SPHERE;

	return (true);
	(void)ray;
	(void)hitRecord;
}

double	Isotropic::scatteringPDF(
	const Ray& ray,
	const HitRecord& hitRecord,
	const Vector3& scatteredDirection
	) const
{
	(void)ray;
	(void)hitRecord;
	(void)scatteredDirection;
	return (1.0 / (4.0 * D_PI));
}

MaterialType	Isotropic::getType(void) const
{
	return (ISOTROPIC);
}
