#include "RendererInternal.hpp"
#include "Defaults.hpp"

// Checks if 'ray' hits objects present 'scene'. On hit, sets 'pixelColor' to the hitted object's material color
bool	Renderer::internal::_checkHits(Scene& scene, Ray& ray, HitRecord& hitRecord)
{
	bool	anyHit = false;
	double	currentClosestObject = T_MAX;
	const auto& accelerationStructure = scene.getAccelerationStructure();

	if (accelerationStructure && accelerationStructure->hit(ray, hitRecord, T_MIN, currentClosestObject))
	{
		currentClosestObject = hitRecord.t0;
		anyHit = true;
	}

	const auto& unacceleratedHittables = scene.getUnacceleratedHittables();
	const auto& hittables = (!accelerationStructure && unacceleratedHittables.empty())
		? scene.getHittables()
		: unacceleratedHittables;

	for (const std::shared_ptr<Hittable>& hittable : hittables)
	{
		if (hittable->hit(ray, hitRecord, T_MIN, currentClosestObject))
		{
			currentClosestObject = hitRecord.t0;
			anyHit = true;
		}
	}

	return (anyHit);
}
