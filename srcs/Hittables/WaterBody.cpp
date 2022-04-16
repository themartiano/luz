#include "Hittables/WaterBody.hpp"
#include "Utilities.hpp"
#include "Materials/Dielectric.hpp"
#include "ImageFiles/Types.hpp"
#include "RefractiveIndexes.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the WaterBody with default values
WaterBody::WaterBody(void)
{
	this->_position = Vector3(0.0, 0.0, 0.0);
	this->_size = 20.0;
	this->_material = std::make_shared<Dielectric>(Color(0.027, 0.1254, 0.2), RI_WATER);
	this->_subSamples = 10;
	this->_noiseScale = 1.0;
	this->_magnitude = 10.0;
	this->_seed = 42;

	this->_perlin = Perlin(this->_seed);
}

// Constructs the WaterBody with custom values
WaterBody::WaterBody(Vector3 position, double size, Color color, unsigned int subSamples, double noiseScale, double magnitude, unsigned int seed)
{
	this->_position = position;
	this->_size = size;
	this->_material = std::make_shared<Dielectric>(color, RI_WATER);
	this->_subSamples = subSamples;
	this->_noiseScale = noiseScale;
	this->_magnitude = magnitude;
	this->_seed = seed;

	this->_perlin = Perlin(this->_seed);
}

// Returns the WaterBody's material
std::shared_ptr<Material>	WaterBody::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the WaterBody's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	WaterBody::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	// get the distance between the origin and the WaterBody closest border
	double closestBorder = Utilities::vectorLength(ray.getOrigin() - (this->_position - (ray.getDirection() * (this->_size / 2.0))));
	double farthestBorder = Utilities::vectorLength(ray.getOrigin() - (this->_position + (ray.getDirection() * (this->_size / 2.0))));

	double stepSize = (farthestBorder - closestBorder) / this->_subSamples;

	// start subsampling in the closest border until the farthest border
	// hits
	for (unsigned int i = 0; i < this->_subSamples; i++)
	{
		double t = closestBorder + (i * stepSize);

		if (t > t_max || t < t_min)
		{
			continue;
		}

		Vector3 samplePosition = ray.pointAtRay(t);

		if (samplePosition.getY() < this->_position.getY())
		{
			continue; // This avoids anything below the WaterBody (position.y)
		}

		if (samplePosition.getX() > this->_position.getX() + (this->_size / 2.0) || samplePosition.getX() < this->_position.getX() - (this->_size / 2.0) ||
			samplePosition.getZ() > this->_position.getZ() + (this->_size / 2.0) || samplePosition.getZ() < this->_position.getZ() - (this->_size / 2.0))
		{
			continue;
		}

		// get the height at the sample position
		double height = this->_getHeightAtPoint(samplePosition.getX(), samplePosition.getZ());

		if (height + this->_position.getY() >= samplePosition.getY())
		{
			hitRecord.t0 = t;
			hitRecord.position = samplePosition;
			hitRecord.normal = this->_getNormalAtPosition(samplePosition, t_min);
			hitRecord.material = this->_material;

			return (true);
		}
	}

	return (false);
}

// Returns the AABB / bounding box for this WaterBody's BVH
bool	WaterBody::createBoundingBox(AABB& outputBoundingBox) const
{
	Vector3 minimum = this->_position - (this->_size / 2.0);
	Vector3 maximum = this->_position + (this->_size / 2.0); // Include the heightset point of the WaterBody

	outputBoundingBox = AABB(minimum, maximum);

	return (true);
}

double	WaterBody::_getHeightAtPoint(double x, double z) const
{
	double n = this->_perlin.noise((x + (this->_size / 2.0)) / this->_noiseScale, 0.0, (z + (this->_size / 2.0)) / this->_noiseScale);

	return (n * this->_magnitude);
}

Vector3	WaterBody::_getNormalAtPosition(Vector3 position, double t_min) const
{
	return (
		Utilities::normalize(Vector3(
			this->_getHeightAtPoint(position.getX() - t_min, position.getZ()) - this->_getHeightAtPoint(position.getX() + t_min, position.getZ()),
			t_min * 2.0,
			this->_getHeightAtPoint(position.getX(), position.getZ() - t_min) - this->_getHeightAtPoint(position.getX(), position.getZ() + t_min)
		))
	);
}
