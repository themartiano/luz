#include "Hittables/Cloud.hpp"
#include "Materials/Lambertian.hpp"
#include "Utilities.hpp"
#include <cmath>

/*
	Constructors
*/

Cloud::Cloud(void)
{
	this->_position = Vector3(0.0, 0.0, 0.0);
	this->_size = 20.0;
	this->_material = std::make_shared<Lambertian>(Color(0.7, 0.7, 0.7));
	this->_samplesPerSizeUnit = 1.0;
	this->_noiseScale = 1.0;
	this->_magnitude = 10.0;
	this->_depth = 0.0;
	this->_seed = 42;

	this->_perlin = Perlin(this->_seed);
}

Cloud::Cloud(Vector3 position, double size, double samplesPerSizeUnit, double noiseScale, double magnitude, double depth, unsigned int seed)
{
	this->_position = position;
	this->_size = size;
	this->_material = std::make_shared<Lambertian>(Color(0.7, 0.7, 0.7));
	this->_samplesPerSizeUnit = samplesPerSizeUnit;
	this->_noiseScale = noiseScale;
	this->_magnitude = magnitude;
	this->_depth = depth;
	this->_seed = seed;

	this->_perlin = Perlin(this->_seed);
}

bool	Cloud::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3 origin = ray.getOrigin();
	// origin.setY(this->_position.getY());
	Vector3 direction = ray.getDirection();
	// direction.setY(this->_position.getY());

	// double closestBorder = (origin * 1).getZ() - (this->_position * 1).getZ() + (this->_size / 2.0);
	// double farthestBorder = (origin * 1).getZ() - (this->_position * 1).getZ() - (this->_size / 2.0);

	// get the distance between the origin and the Cloud closest border
	double closestBorder = Utilities::vectorLength(origin - this->_position) + (direction * (this->_size / 2.0)).getZ();
	double farthestBorder = Utilities::vectorLength(origin - this->_position) + (direction * (this->_size / 2.0)).getZ();

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

		if (samplePosition.getX() > this->_position.getX() + (this->_size / 2.0) || samplePosition.getX() < this->_position.getX() - (this->_size / 2.0) ||
			samplePosition.getZ() > this->_position.getZ() + (this->_size / 2.0) || samplePosition.getZ() < this->_position.getZ() - (this->_size / 2.0))
		{
			continue;
		}

		// get the height at the sample position
		double height = this->_getHeightAtPoint2(this->_position - samplePosition);

		// if (samplePosition.getY() < this->_position.getY() - this->_depth)
		// {
		// 	continue;
		// }

		if (height >= fabs(samplePosition.getY() - this->_position.getY()))
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

double Cloud::sla(double x, double y, double z) const
{
	double noiseSum = 0.0, frequency = 1.0, amplitude = 1.0;

	for (int i = 0; i < 4; i++)
	{
		noiseSum += this->_perlin.noise(x * frequency, y * frequency, z * frequency) * amplitude;
		amplitude /= 2.0;
		frequency *= 2.0;
	}

	return (noiseSum);
}

double	Cloud::_getHeightAtPoint2(Vector3 point) const
{
	double x = point.getX() - (this->_size / 2.0);
	double y = point.getY() - (10.0 / 2.0); // 10.0 == height
	double z = point.getZ() - (this->_size / 2.0);

	double n = 0.7 * sla(x, y, z);
	// std::cout << "height: " << n << std::endl;
	return (n);
}
