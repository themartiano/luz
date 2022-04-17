#include "Hittables/Procedural.hpp"
#include "Utilities.hpp"
#include "Materials/Dielectric.hpp"
#include "ImageFiles/Types.hpp"
#include "RefractiveIndexes.hpp"
#include <cmath>

/*
	Constructors
*/

// Returns the Procedural's material
std::shared_ptr<Material>	Procedural::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the Procedural's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Procedural::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3 origin = ray.getOrigin();
	origin.setY(this->_position.getY());
	Vector3 direction = ray.getDirection();
	direction.setY(this->_position.getY());

	// double closestBorder = (origin * 1).getZ() - (this->_position * 1).getZ() + (this->_size / 2.0);
	// double farthestBorder = (origin * 1).getZ() - (this->_position * 1).getZ() - (this->_size / 2.0);

	// get the distance between the origin and the Procedural closest border
	double closestBorder = Utilities::vectorLength((origin * direction) - (this->_position + (direction * (this->_size / 2.0)))) * -1;
	double farthestBorder = Utilities::vectorLength((origin * direction) - (this->_position - (direction * (this->_size / 2.0))));

	// std::cout << "closestBorder:" << closestBorder << std::endl;
	// std::cout << "farthestBorder:" << farthestBorder << std::endl;

	if (closestBorder < 0.0)
	{
		closestBorder = 0.0;
	}
	if (farthestBorder < 0.0)
	{
		farthestBorder = 0.0;
	}

	unsigned int totalSamples = this->_samplesPerSizeUnit * this->_size;
	double stepSize = this->_size / totalSamples;

	// start subsampling in the closest border until the farthest border
	// hits
	for (unsigned int i = 0; i < totalSamples; i++)
	{
		double t = closestBorder + (i * stepSize);

		if (t > t_max || t < t_min)
		{
			continue;
		}

		Vector3 samplePosition = ray.pointAtRay(t);

		if (samplePosition.getY() < this->_position.getY() - this->_depth)
		{
			continue; // This avoids anything below the landscape (position.y)
		}

		if (samplePosition.getX() > this->_position.getX() + (this->_size / 2.0) || samplePosition.getX() < this->_position.getX() - (this->_size / 2.0) ||
			samplePosition.getZ() > this->_position.getZ() + (this->_size / 2.0) || samplePosition.getZ() < this->_position.getZ() - (this->_size / 2.0))
		{
			continue;
		}

		// get the height at the sample position
		double height = this->_getHeightAtPoint(samplePosition.getX() - this->_position.getX(), samplePosition.getZ() - this->_position.getZ());

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

// Returns the AABB / bounding box for this Procedural's BVH
bool	Procedural::createBoundingBox(AABB& outputBoundingBox) const
{
	Vector3 minimum = this->_position - (this->_size / 2.0);
	Vector3 maximum = this->_position + (this->_size / 2.0); // Include the heightset point of the Procedural

	outputBoundingBox = AABB(minimum, maximum);

	return (true);
}

double	Procedural::_getHeightAtPoint(double localX, double localZ) const
{
	double x = fabs(localX - (this->_size / 2.0));
	double z = fabs(localZ - (this->_size / 2.0));

	double n = this->_perlin.noise(x / this->_noiseScale, 0.0, z / this->_noiseScale);

	return (n * this->_magnitude);
}

Vector3	Procedural::_getNormalAtPosition(Vector3 position, double t_min) const
{
	return (
		Utilities::normalize(Vector3(
			this->_getHeightAtPoint(position.getX() - t_min, position.getZ()) - this->_getHeightAtPoint(position.getX() + t_min, position.getZ()),
			t_min * 2.0,
			this->_getHeightAtPoint(position.getX(), position.getZ() - t_min) - this->_getHeightAtPoint(position.getX(), position.getZ() + t_min)
		))
	);
}
