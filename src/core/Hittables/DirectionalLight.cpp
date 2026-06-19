#include "Hittables/DirectionalLight.hpp"
#include "Defaults.hpp"
#include "Materials/Emissive.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <stdexcept>

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
	this->_material = std::make_shared<Emissive>(Color(1.0, 1.0, 1.0));
	this->_hasAtmosphereSunRadiance = false;
	this->_atmosphereSunRadiance = Color(0.0, 0.0, 0.0);
}

DirectionalLight::DirectionalLight(Vector3 direction, std::shared_ptr<Material> material)
{
	this->_direction = normalizedLightDirection(direction);
	this->_material = material;
	this->_hasAtmosphereSunRadiance = false;
	this->_atmosphereSunRadiance = Color(0.0, 0.0, 0.0);
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

Vector3	DirectionalLight::getDirection(void) const
{
	return (this->_direction);
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

void	DirectionalLight::setAtmosphereSunRadiance(Color radiance)
{
	if (
		!std::isfinite(radiance.getRed())
		|| !std::isfinite(radiance.getGreen())
		|| !std::isfinite(radiance.getBlue())
		|| radiance.getRed() < 0.0
		|| radiance.getGreen() < 0.0
		|| radiance.getBlue() < 0.0
	)
	{
		throw std::invalid_argument("Atmosphere sun radiance must be finite and non-negative.");
	}
	this->_atmosphereSunRadiance = radiance;
	this->_hasAtmosphereSunRadiance = true;
}

bool	DirectionalLight::hasAtmosphereSunRadiance(void) const
{
	return (this->_hasAtmosphereSunRadiance);
}

Color	DirectionalLight::getAtmosphereSunRadiance(void) const
{
	return (this->_atmosphereSunRadiance);
}
