#include "Renderer/Renderer.hpp"
#include "Defaults.hpp"

// Checks if 'ray' hits objects present 'scene'. On hit, sets 'pixelColor' to the hitted object's material color
bool	Renderer::internal::_checkHits(Scene& scene, Ray& ray, HitRecord& hitRecord)
{
	bool	anyHit = false;
	double	currentClosestObject = T_MAX;
	static	std::vector<std::shared_ptr<Hittable>> hittables = scene.getHittables();

	for (std::shared_ptr<Hittable> hittable : hittables)
	{
		if (hittable->hit(ray, hitRecord, T_MIN, currentClosestObject))
		{
			// if (hitRecord.t0 > T_MIN)
			// {
				currentClosestObject = hitRecord.t0;
				anyHit = true;
			// }
		}
	}

	return (anyHit);
}
