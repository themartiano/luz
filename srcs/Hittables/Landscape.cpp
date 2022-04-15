#include "Hittables/Landscape.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include "ImageFiles/Types.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Landscape with default values
Landscape::Landscape(void)
{
	this->_position = Vector3(0.0, 0.0, 0.0);
	this->_size = 20.0;
	this->_material = std::make_shared<Lambertian>(Color(0.3, 0.29, 0.11));
	this->_subSamples = 10;
	this->_noiseScale = 1.0;
	this->_seed = 42;

	this->_perlin = Perlin(this->_seed);
}

// Constructs the Landscape with custom values
Landscape::Landscape(Vector3 position, double size, std::shared_ptr<Material> material, unsigned int subSamples, double noiseScale, unsigned int seed)
{
	this->_position = position;
	this->_size = size;
	this->_material = material;
	this->_subSamples = subSamples;
	this->_noiseScale = noiseScale;
	this->_seed = seed;

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
	// get the distance between the origin and the landscape closest border
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
			continue; // This avoids anything below the landscape (position.y)
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

// bool	Landscape::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
// {
// 	Vector3	position(0.0, 0.0, 0.0); // Option to set custom
// 	Vector3	orientation(0.0, 1.0, 0.0); // Option to set custom

// 	double	tCurrent = t_min;

// 	double size = 100.0;

// 	(void)t_max;

// 	double a = Utilities::dot(ray.getOrigin() - position, orientation);
// 	double b = Utilities::dot(ray.getDirection(), orientation);
// 	double t = -a / b;


// 	double segmentLength = (t + (size / 2.0)) / this->_subSamples;

// 	for (unsigned int i = 0; i < this->_subSamples; i++)
// 	{
// 		Vector3 samplePosition = ray.getOrigin() + ((tCurrent + segmentLength * 0.5) * ray.getDirection());

// 		double	landscapeHeight = this->_getHeightAtPoint(samplePosition.getX(), samplePosition.getZ());

// 		if (landscapeHeight >= samplePosition.getY())
// 		{
// 			hitRecord.t0 = Utilities::vectorLength(ray.getOrigin() - samplePosition); // check this
// 			hitRecord.normal = this->_getNormalAtPosition(samplePosition, t_min);
// 			hitRecord.position = samplePosition;
// 			hitRecord.material = this->_material;

// 			// calculate normal at samplePosition

// 			// Perlin p(this->_seed);
// 			// double n = p.noise(fabs(hitRecord.position.getX()) / this->_noiseScale, fabs(hitRecord.position.getY()) / this->_noiseScale, fabs(hitRecord.position.getZ()) / this->_noiseScale);

// 			// hitRecord.material = std::make_shared<Lambertian>(Color(n * this->_material->getColor().getRed(), n * this->_material->getColor().getGreen(), n * this->_material->getColor().getBlue()));

// 			return (true);
// 		}

// 		tCurrent += segmentLength;
// 	}

// 	return (false);
// }

// bool	Landscape::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
// {
// 	Vector3	position(0.0, 0.0, 0.0); // Option to set custom
// 	Vector3	orientation(0.0, 1.0, 0.0); // Option to set custom

// 	double	tCurrent = t_min;

// 	(void)t_max;

// 	// double a = Utilities::dot(ray.getOrigin() - position, orientation);
// 	// double b = Utilities::dot(ray.getDirection(), orientation);
// 	// if (b == 0.0 || (a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0))
// 	// {
// 	// 	// return (false);
// 	// }

// 	// double t = -a / b;
// 	// if (t > t_max || t < t_min)
// 	// {
// 	// 	// return (false);
// 	// }

// 	// Vector3 d = ray.pointAtRay(t) - position;
// 	// if (fabs(d.getX()) > (this->_size) || fabs(d.getZ()) > (this->_size))
// 	// {
// 	// 	return (false);
// 	// }

// 	double segmentLength = 1.0;//(t + (size / 2.0)) / this->_subSamples;

// 	if (segmentLength <= 0.0)
// 	{
// 		segmentLength = t_min;
// 	}

// 	tCurrent -= segmentLength;
// 	while (tCurrent < this->_size)
// 	{
// 		tCurrent += segmentLength;

// 		// Only samples the ray when it's inside the landscape
// 		Vector3 samplePosition = ray.getOrigin() + tCurrent * ray.getDirection();
// 		//(ray.getOrigin() - (position - (ray.getDirection() * (this->_size / 2.0))))

// 		// double t = Utilities::vectorLength(ray.getOrigin() - samplePosition);
// 		// if (t < t_min || t > t_max)
// 		// {
// 		// 	continue;
// 		// }

// 		double	landscapeHeight = this->_getHeightAtPoint((samplePosition.getX() - position.getX()), (samplePosition.getZ() - position.getZ()));
// 		// std::cout << (samplePosition.getY() - position.getY()) << std::endl;

// 		// std::cout << "position x: " << position.getX() << std::endl;
// 		// std::cout << "sample x: " << samplePosition.getX() << std::endl;
// 		// std::cout << "sample x - position x: " << samplePosition.getX() - position.getX() << std::endl;
// 		// std::cout << "landscape height: " << landscapeHeight << std::endl;

// 		if (position.getY() + landscapeHeight >= samplePosition.getY())
// 		{
// 			if (fabs(samplePosition.getX() - position.getX()) > (this->_size / 2.0) || fabs(samplePosition.getZ() - position.getZ()) > (this->_size / 2.0))
// 			{
// 				continue;
// 			}

// 			hitRecord.t0 = tCurrent - 0.5 * segmentLength; // check this
// 			hitRecord.normal = this->_getNormalAtPosition(samplePosition, t_min);
// 			hitRecord.position = samplePosition;
// 			hitRecord.material = this->_material;

// 			// calculate normal at samplePosition

// 			// Perlin p(this->_seed);
// 			// double n = p.noise(
// 			// 	((hitRecord.position.getX() - position.getX()) + (this->_size / 2.0)) / this->_noiseScale,
// 			// 	((hitRecord.position.getY() - position.getY()) + (this->_size / 2.0)) / this->_noiseScale,
// 			// 	((hitRecord.position.getZ() - position.getZ()) + (this->_size / 2.0)) / this->_noiseScale);

// 			// hitRecord.material = std::make_shared<Lambertian>(Color(n * this->_material->getColor().getRed(), n * this->_material->getColor().getGreen(), n * this->_material->getColor().getBlue()));

// 			return (true);
// 		}
// 	}

// 	return (false);
// }

// Returns the AABB / bounding box for this Landscape's BVH
bool	Landscape::createBoundingBox(AABB& outputBoundingBox) const
{
	// this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
	(void)outputBoundingBox;
}

double	Landscape::_getHeightAtPoint(double x, double z) const
{
	double n = this->_perlin.noise((x + (this->_size / 2.0)) / this->_noiseScale, 0.0, (z + (this->_size / 2.0)) / this->_noiseScale);

	return (n * 10);
}

// double	Landscape::_getHeightAtPoint(double x, double z) const
// {
// 	double n = this->_perlin.noise(x / this->_noiseScale, 0.0, z / this->_noiseScale);

// 	return (n * 10);
// }

Vector3	Landscape::_getNormalAtPosition(Vector3 position, double t_min) const
{
	return (
		Utilities::normalize(Vector3(
			this->_getHeightAtPoint(position.getX() - t_min, position.getZ()) - this->_getHeightAtPoint(position.getX() + t_min, position.getZ()),
			t_min * 2.0,
			this->_getHeightAtPoint(position.getX(), position.getZ() - t_min) - this->_getHeightAtPoint(position.getX(), position.getZ() + t_min)
		))
	);
}
