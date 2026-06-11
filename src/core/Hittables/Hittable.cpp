#include "Hittables/Hittable.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>

bool Hittable::hitAny(Ray& ray, double t_min, double t_max) const
{
	HitRecord hitRecord;

	return (this->hit(ray, hitRecord, t_min, t_max));
}

double Hittable::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	return (0.0);
	(void)origin;
	(void)vec;
}

Vector3 Hittable::random(const Vector3& origin) const
{
	return (Vector3(1.0, 0.0, 0.0));
	(void)origin;
}

bool Hittable::sampleLight(const Vector3& origin, HittableLightSample& sample) const
{
	sample = HittableLightSample();
	sample.direction = this->random(origin);

	const double directionLengthSquared = Utilities::vectorLengthSquared(sample.direction);
	if (!std::isfinite(directionLengthSquared) || directionLengthSquared <= 0.0)
	{
		return (false);
	}

	Ray lightRay(origin, sample.direction);
	HitRecord hitRecord;
	if (!this->hit(lightRay, hitRecord, T_MIN, T_MAX))
	{
		return (false);
	}

	sample.pdf = this->pdfValue(origin, sample.direction);
	sample.tMax = hitRecord.t0;
	sample.material = hitRecord.material;
	if (
		sample.pdf <= 0.0
		|| sample.tMax <= T_MIN
		|| !std::isfinite(sample.pdf)
		|| !std::isfinite(sample.tMax)
	)
	{
		return (false);
	}

	sample.valid = true;
	return (true);
}

double Hittable::lightSelectionWeight(void) const
{
	const std::shared_ptr<Material> material = this->getMaterial();
	if (!material)
	{
		return (0.0);
	}

	const double luminance = Utilities::luminance(material->emitted());
	if (!std::isfinite(luminance) || luminance <= 0.0)
	{
		return (0.0);
	}
	return (luminance);
}
