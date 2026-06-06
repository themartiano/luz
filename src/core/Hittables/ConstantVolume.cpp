#include "Hittables/ConstantVolume.hpp"
#include "Hittables/Sphere.hpp"
#include "Materials/Isotropic.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Sampler.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the ConstantVolume with default values
ConstantVolume::ConstantVolume(void)
{
	this->_boundary = std::make_shared<Sphere>(
		Vector3(0.0, 0.0, 0.0),
		6.0,
		nullptr
	);
	this->_phaseFunction = std::make_shared<Isotropic>(Color(0.6, 0.6, 0.6));
	this->_negativeInverseDensity = -1.0 / D_VOLUME_DENSITY;
}

// Constructs the ConstantVolume with custom values
ConstantVolume::ConstantVolume(std::shared_ptr<Hittable> boundary, std::shared_ptr<Material> phaseFunction, double density)
{
	this->_boundary = boundary;
	this->_phaseFunction = phaseFunction;
	this->_negativeInverseDensity = -1.0 / density;
}

// Returns the ConstantVolume's material
std::shared_ptr<Material>	ConstantVolume::getMaterial(void) const
{
	return (this->_phaseFunction);
}

// Calculates if the ConstantVolume is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	ConstantVolume::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	HitRecord hitRecord1, hitRecord2;

	if (!this->_boundary->hit(ray, hitRecord1, t_min, t_max))
	{
		return (false);
	}

	if (!this->_boundary->hit(ray, hitRecord2, hitRecord1.t0 + t_min, t_max))
	{
		return (false);
	}

	if (hitRecord1.t0 < t_min)
	{
		hitRecord1.t0 = t_min;
	}
	if (hitRecord2.t0 > t_max)
	{
		hitRecord2.t0 = t_max;
	}

	if (hitRecord1.t0 >= hitRecord2.t0)
	{
		return (false);
	}

	if (hitRecord1.t0 < 0)
	{
		hitRecord1.t0 = 0;
	}

	double rayLength = Utilities::vectorLength(ray.getDirection());
	double distanceInsideBoundary = (hitRecord2.t0 - hitRecord1.t0) * rayLength;
	double hitDistance = this->_negativeInverseDensity * log(std::max(Sampler::sample1D(Sampler::DIM_VOLUME_DISTANCE), 1e-12));

	if (hitDistance > distanceInsideBoundary)
	{
		return (false);
	}

	hitRecord.t0 = hitRecord1.t0 + hitDistance / rayLength;
	hitRecord.position = ray.pointAtRay(hitRecord.t0);
	hitRecord.normal = Vector3(1.0, 0.0, 0.0); // Arbitrary
	hitRecord.material = this->_phaseFunction;

	return (true);
}

// Creates an AABB / bounding box for this ConstantVolume
bool	ConstantVolume::createBoundingBox(AABB& outputBoundingBox) const
{
	return (this->_boundary->createBoundingBox(outputBoundingBox));
}
