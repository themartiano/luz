#include "Hittables/Landscape.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Landscape with default values
Landscape::Landscape(void)
{
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_noiseScale = 1.0;
	this->_seed = 42;
	this->_samplesPerRay = 10;

	this->_perlin = Perlin(this->_seed);
}

// Constructs the Landscape with custom values
Landscape::Landscape(std::shared_ptr<Material> material, double noiseScale, unsigned int seed, unsigned int samplesPerRay)
{
	this->_material = material;
	this->_noiseScale = noiseScale;
	this->_seed = seed;
	this->_samplesPerRay = samplesPerRay;

	this->_perlin = Perlin(this->_seed);
}

// Returns the Landscape's material
std::shared_ptr<Material>	Landscape::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the Landscape's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Landscape::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3	position(0.0, 0.0, 0.0); // Option to set custom
	Vector3	orientation(0.0, 1.0, 0.0); // Option to set custom

	double	segmentLength = 1.0; // Option to set custom

	double	tCurrent = t_min;

	(void)t_max;

	for (unsigned int i = 0; i < this->_samplesPerRay; i++)
	{
		Vector3 samplePosition = ray.getOrigin() + ((tCurrent + segmentLength * 0.5) * ray.getDirection());
		// std::cout << "height at (" << samplePosition.getX() << ", " << samplePosition.getZ() << ") = " << this->_getHeightAtPoint(samplePosition.getX(), samplePosition.getZ()) << std::endl;

		double	landscapeHeight = this->_getHeightAtPoint(samplePosition.getX(), samplePosition.getZ());

		if (landscapeHeight >= samplePosition.getY())
		{
			hitRecord.t0 = Utilities::vectorLength(ray.getOrigin() - samplePosition); // check this
			hitRecord.normal = orientation;
			hitRecord.position = samplePosition;
			hitRecord.material = this->_material;

			// Perlin p(this->_seed);
			// double n = p.noise(fabs(hitRecord.position.getX()) / this->_noiseScale, fabs(hitRecord.position.getY()) / this->_noiseScale, fabs(hitRecord.position.getZ()) / this->_noiseScale);

			// hitRecord.material = std::make_shared<Lambertian>(Color(n * this->_material->getColor().getRed(), n * this->_material->getColor().getGreen(), n * this->_material->getColor().getBlue()));

			return (true);
		}

		tCurrent += segmentLength;
	}

	return (false);

	// double d = Utilities::dot(ray.getDirection(), orientation);
	// if (d == 0.0)
	// {
	// 	return (false);
	// }

	// double t = Utilities::dot(position - ray.getOrigin(), orientation) / d;
	// if (t > t_max || t < t_min)
	// {
	// 	return (false);
	// }

	// hitRecord.t0 = t;
	// hitRecord.normal = orientation;
	// hitRecord.position = ray.pointAtRay(t);

	// Perlin p(this->_seed);
	// double n = p.noise(fabs(hitRecord.position.getX()) / this->_noiseScale, fabs(hitRecord.position.getY()) / this->_noiseScale, fabs(hitRecord.position.getZ()) / this->_noiseScale);

	// hitRecord.material = std::make_shared<Lambertian>(Color(n * this->_material->getColor().getRed(), n * this->_material->getColor().getGreen(), n * this->_material->getColor().getBlue()));

	// // hitRecord.position = Vector3(hitRecord.position.getX(), hitRecord.position.getY() + n, hitRecord.position.getZ());

	// return (true);
}

// Returns the AABB / bounding box for this Landscape's BVH
bool	Landscape::createBoundingBox(AABB& outputBoundingBox) const
{
	// this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
	(void)outputBoundingBox;
}

double	Landscape::_getHeightAtPoint(double x, double z) const
{
	double n = this->_perlin.noise(fabs(x) / this->_noiseScale, 0.0, fabs(z) / this->_noiseScale);

	return (n * 10);
}
