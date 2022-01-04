#include "Renderer/Renderer.hpp"
#include "Utilities.hpp"
#include "RefractiveIndexes.hpp"
#include "PDFs/MixturePDF.hpp"

// Calculates the light rays bounce/reflection direction
void	Renderer::_calculateLightRayBounceDirection(Ray& ray, Color& color, const MixturePDF& pdf)
{
	if (ray.hitRecord.material.getMetallic() == 1.0)
	{
		ray.setDirection(Utilities::reflect(ray.getDirection(), ray.hitRecord.normal) + (Utilities::randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
		return;
	}

	if (ray.hitRecord.material.getIsDielectric() == true)
	{
		Vector3	refractedVector;
		Vector3	outwardsNormal;
		double	reflectionProbability;
		double	cosine;
		double	refractiveIndex = RI_GLASS;
		double	directionNormalDot = Utilities::dot(ray.getDirection(), ray.hitRecord.normal);

		if (directionNormalDot > 0.0)
		{
			outwardsNormal = ray.hitRecord.normal * -1.0;
			cosine = refractiveIndex * directionNormalDot / Utilities::vectorLength(ray.getDirection());
		}
		else
		{
			outwardsNormal = ray.hitRecord.normal;
			refractiveIndex = 1.0 / refractiveIndex;
			cosine = -directionNormalDot / Utilities::vectorLength(ray.getDirection());
		}

		if (Utilities::refract(ray.getDirection(), outwardsNormal, refractiveIndex, refractedVector))
		{
			reflectionProbability = Utilities::schlick(cosine, refractiveIndex);
		}
		else
		{
			reflectionProbability = 1.0;
		}

		if (Utilities::randomDouble() < reflectionProbability)
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(Utilities::reflect(ray.getDirection(), ray.hitRecord.normal));
		}
		else
		{
			ray.setOrigin(ray.hitRecord.position);
			ray.setDirection(refractedVector);
		}

		color = Color(1.0, 1.0, 1.0);
		return;
	}

	Vector3	newTarget = ray.hitRecord.position + ray.hitRecord.normal + Utilities::randomPointInsideUnitSphere();
	if (ray.hitRecord.material.getMetallic() == 0.0)
	{
		ray.setDirection(pdf.generate());
		color = ray.hitRecord.material.getColor() * ray.hitRecord.material.getAlbedo();
		return;
	}

	if (Utilities::randomDouble() < ray.hitRecord.material.getMetallic())
	{
		ray.setDirection(Utilities::reflect(ray.getDirection(), ray.hitRecord.normal) + (Utilities::randomPointInsideUnitSphere() * ray.hitRecord.material.getReflectionFuzziness()));
	}
	else
	{
		ray.setDirection(newTarget - ray.hitRecord.position);
	}
}