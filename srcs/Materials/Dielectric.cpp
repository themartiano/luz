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
	scatterRecord.attenuation = Color(1.0, 1.0, 1.0);

	double	refractionRatio = RI_GLASS;
	if (Utilities::dot(ray.getDirection(), hitRecord.normal) <= 0.0)
	{
		refractionRatio = 1.0 / refractionRatio;
	}

	double	cosTheta = fmin(Utilities::dot(ray.getDirection() * -1.0, hitRecord.normal), 1.0);
	double	sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	bool	cannotRefract = refractionRatio * sinTheta > 1.0;

	Vector3	direction;
	if (cannotRefract || Utilities::schlick(cosTheta, refractionRatio) > Utilities::randomDouble())
	{
		direction = Utilities::reflect(ray.getDirection(), hitRecord.normal);
	}
	else
	{
		direction = Utilities::refract(ray.getDirection(), hitRecord.normal, refractionRatio);
	}

	scatterRecord.specularRay = Ray(hitRecord.position, direction);

	return (true);
}

MaterialType	Dielectric::getType(void) const
{
	return (DIELECTRIC);
}
