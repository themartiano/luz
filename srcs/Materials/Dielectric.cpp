#include "Materials/Dielectric.hpp"
#include "Utilities.hpp"
#include "RefractiveIndexes.hpp"
#include <cmath>

Dielectric::Dielectric(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
}

Dielectric::Dielectric(Color color)
{
	this->_color = color;
}

bool	Dielectric::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	scatterRecord.isSpecular = true;
	scatterRecord.pdfPtr = nullptr;
	scatterRecord.attenuation = this->_color;

	double	refractionRatio = RI_GLASS;
	Vector3	normalizedDirection = Utilities::normalize(ray.getDirection());
	if (Utilities::dot(normalizedDirection, hitRecord.normal) <= 0.0)
	{
		refractionRatio = 1.0 / refractionRatio;
	}

	double	cosTheta = fmin(Utilities::dot(normalizedDirection * -1.0, hitRecord.normal), 1.0);
	double	sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	bool	cannotRefract = refractionRatio * sinTheta > 1.0;

	Vector3	direction;
	if (cannotRefract || Utilities::schlick(cosTheta, refractionRatio) > Utilities::randomDouble())
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
