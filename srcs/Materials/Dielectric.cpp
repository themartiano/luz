#include "Materials/Dielectric.hpp"
#include "Utilities.hpp"
#include "RefractiveIndexes.hpp"
#include <cmath>

bool	Dielectric::scatter(Ray& ray)
{
	ray.scatterRecord.isSpecular = true;
	ray.scatterRecord.pdfPtr = nullptr;
	ray.scatterRecord.attenuation = Color(1.0, 1.0, 1.0);

	double	refractionRatio = RI_GLASS;
	if (Utilities::dot(ray.getDirection(), ray.hitRecord.normal) <= 0.0)
	{
		refractionRatio = 1.0 / refractionRatio;
	}

	double	cosTheta = fmin(Utilities::dot(ray.getDirection() * -1.0, ray.hitRecord.normal), 1.0);
	double	sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	bool	cannotRefract = refractionRatio * sinTheta > 1.0;

	Vector3	direction;
	if (cannotRefract || Utilities::schlick(cosTheta, refractionRatio) > Utilities::randomDouble())
	{
		direction = Utilities::reflect(ray.getDirection(), ray.hitRecord.normal);
	}
	else
	{
		direction = Utilities::refract(ray.getDirection(), ray.hitRecord.normal, refractionRatio);
	}

	ray.scatterRecord.specularRay = std::make_unique<Ray>(ray.hitRecord.position, direction);

	return (true);
}
