#include "Hittables/DirectionalLight.hpp"
#include "Defaults.hpp"
#include "Materials/Emissive.hpp"
#include "Utilities.hpp"
#include <cmath>

namespace
{
	Vector3	normalizedLightDirection(const Vector3& direction)
	{
		if (Utilities::vectorLengthSquared(direction) <= 0.0)
		{
			return (Vector3(0.0, -1.0, 0.0));
		}
		return (Utilities::normalize(direction));
	}
}

DirectionalLight::DirectionalLight(void)
{
	this->_direction = Vector3(0.0, -1.0, 0.0);
	this->_material = std::make_shared<Emissive>(Color(1.0, 1.0, 1.0), 1.0);
}

DirectionalLight::DirectionalLight(Vector3 direction, std::shared_ptr<Material> material)
{
	this->_direction = normalizedLightDirection(direction);
	this->_material = material;
}

bool	DirectionalLight::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	(void)ray;
	(void)hitRecord;
	(void)t_min;
	(void)t_max;
	return (false);
}

bool	DirectionalLight::createBoundingBox(AABB& outputBoundingBox) const
{
	(void)outputBoundingBox;
	return (false);
}

Material*	DirectionalLight::getMaterial(void) const
{
	return (this->_material.get());
}

double	DirectionalLight::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	(void)origin;
	(void)vec;
	return (0.0);
}

Vector3	DirectionalLight::random(const Vector3& origin) const
{
	(void)origin;
	return (this->_direction * -1.0);
}

bool	DirectionalLight::sampleLight(const Vector3& origin, HittableLightSample& sample) const
{
	(void)origin;
	sample = HittableLightSample();
	sample.direction = this->random(origin);
	sample.pdf = 1.0;
	sample.tMax = T_MAX;
	sample.material = this->_material.get();
	sample.valid = (
		this->_material != nullptr
		&& Utilities::vectorLengthSquared(sample.direction) > 0.0
		&& std::isfinite(sample.pdf)
	);
	return (sample.valid);
}

double	DirectionalLight::lightSelectionWeight(void) const
{
	if (!this->_material)
	{
		return (0.0);
	}

	const double luminance = Utilities::luminance(this->_material->emitted());
	if (!std::isfinite(luminance) || luminance <= 0.0)
	{
		return (0.0);
	}
	return (luminance);
}
