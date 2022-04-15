#include "Hittables/Landscape.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include "Noise/Perlin.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Landscape with default values
Landscape::Landscape(void)
{
	this->_transform = Transform(Vector3(), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0));
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_width = 100.0;
	this->_length = 100.0;
	this->_noiseScale = 1.0;
	this->_seed = 42;
}

// Constructs the Landscape with custom values
Landscape::Landscape(Transform transform, std::shared_ptr<Material> material, double width, double length, double noiseScale, unsigned int seed)
{
	this->_transform = transform;
	this->_material = material;
	this->_width = width;
	this->_length = length;
	this->_noiseScale = noiseScale;
	this->_seed = seed;
}

// Returns the Landscape's material
std::shared_ptr<Material>	Landscape::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the Landscape's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Landscape::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	double a = Utilities::dot(ray.getOrigin() - this->_transform.getPosition(), this->_transform.getOrientation());
	double b = Utilities::dot(ray.getDirection(), this->_transform.getOrientation());
	if (b == 0.0 || (a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0))
	{
		return (false);
	}

	double t = -a / b;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	Vector3 d = ray.pointAtRay(t) - this->_transform.getPosition();
	if (fabs(this->_transform.getOrientation().getY()) > 0.0)
	{
		if (fabs(d.getX()) > (this->_width / 2.0) || fabs(d.getZ()) > (this->_length / 2.0))
		{
			return (false);
		}
	}
	else if (fabs(this->_transform.getOrientation().getZ()) > 0.0)
	{
		if (fabs(d.getX()) > (this->_width / 2.0) || fabs(d.getY()) > (this->_length / 2.0))
		{
			return (false);
		}
	}
	else if (fabs(this->_transform.getOrientation().getX()) > 0.0)
	{
		if (fabs(d.getZ()) > (this->_width / 2.0) || fabs(d.getY()) > (this->_length / 2.0))
		{
			return (false);
		}
	}

	hitRecord.t0 = t;
	hitRecord.normal = this->_transform.getOrientation();
	hitRecord.position = ray.pointAtRay(t);

	Perlin p(this->_seed);
	double n = p.noise(fabs(hitRecord.position.getX()) / this->_noiseScale, fabs(hitRecord.position.getY()) / this->_noiseScale, fabs(hitRecord.position.getZ()) / this->_noiseScale);

	hitRecord.material = std::make_shared<Lambertian>(Color(n * this->_material->getColor().getRed(), n * this->_material->getColor().getGreen(), n * this->_material->getColor().getBlue()));

	return (true);
}

// Returns the AABB / bounding box for this Landscape's BVH
bool	Landscape::createBoundingBox(AABB& outputBoundingBox) const
{
	// this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
	(void)outputBoundingBox;
}
