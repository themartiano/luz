#include "Materials/Isotropic.hpp"
#include "Utilities.hpp"
#include "PDFs/SpherePDF.hpp"
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
	scatterRecord.attenuation = this->_color;
	scatterRecord.pdfPtr = std::make_shared<SpherePDF>();

	return (true);
	(void)ray;
	(void)hitRecord;
}

double	Isotropic::scatteringPDF(Ray& ray, HitRecord& hitRecord)
{
	return (1.0 / (4.0 * D_PI));
	(void)ray;
	(void)hitRecord;
}

MaterialType	Isotropic::getType(void) const
{
	return (ISOTROPIC);
}
