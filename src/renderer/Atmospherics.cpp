#include "RendererInternal.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Sampler.hpp"

// Computes the atmosphere color
Color	Renderer::internal::_computeAtmosphereColor(Scene& scene, Ray& ray)
{
	// If the Earth radius is not added, the origin will be inside the Earth
	Ray atmosphereRay(ray.getOrigin(), ray.getDirection());
	HitRecord atmosphereHitRecord;
	const Atmosphere& atmosphere = scene.getAtmosphere();

	double t_max = T_MAX;

	// Checks ray collisions with Earth
	if (planetaryHit(atmosphere.getEarthRadius(), atmosphereRay, atmosphereHitRecord) && atmosphereHitRecord.t1 > 0.0)
	{
		t_max = std::max(0.0, atmosphereHitRecord.t0);
	}

	Color atmosphereColor = atmosphere.computeIncidentLight(atmosphereRay, atmosphereHitRecord, t_max);

	double random = Sampler::sample1D(Sampler::DIM_ATMOSPHERE);
	if (random >= 0.9996)
	{
		double starSample = Sampler::sample1D(Sampler::DIM_ATMOSPHERE + 1);
		double diff = (atmosphere.getStarsBrightness() - 0.2 + (0.4 * starSample)) - ((atmosphereColor.getRed() + atmosphereColor.getGreen() + atmosphereColor.getBlue()) / 3.0);
		if (diff < 0.0)
		{
			diff = 0.0;
		}
		else if (diff > 1.0)
		{
			diff = 1.0;
		}
		atmosphereColor += Color(diff, diff, diff);
	}
	return (atmosphereColor);
}

// Calculates the sky interpolation for the background and reflexes
Color	Renderer::internal::_calculateSkyInterpolation(Scene& scene, Ray& ray)
{
	const double	skyLine = scene.getSkyline();

	Vector3	normalizedDirection = Utilities::normalize(ray.getDirection());

	double temp = skyLine * (normalizedDirection.getY() + 1.0);

	return ((Color(1.0, 1.0, 1.0) * (1.0 - temp)) + (Color(0.5, 0.7, 1.0) * temp));
}
