#include "Hittables/Landscape.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"

/*
	Constructors
*/

// Constructs the Landscape with default values
Landscape::Landscape(void)
{
	this->_transform = Transform(Vector3(), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0));
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_seed = 42;
}

// Constructs the Landscape with custom values
Landscape::Landscape(Transform transform, std::shared_ptr<Material> material, unsigned int seed)
{
	this->_transform = transform;
	this->_material = material;
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
	Vector3 position(0.0, this->_transform.getPosition().getY(), 0.0);

	double d = Utilities::dot(ray.getDirection(), this->_transform.getOrientation());
	if (d == 0.0)
	{
		return (false);
	}

	double t = Utilities::dot(position - ray.getOrigin(), this->_transform.getOrientation()) / d;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	hitRecord.t0 = t;
	hitRecord.normal = this->_transform.getOrientation();
	hitRecord.material = this->_material;
	hitRecord.position = ray.pointAtRay(t);

	return (true);
}

// Returns the AABB / bounding box for this Landscape's BVH
bool	Landscape::createBoundingBox(AABB& outputBoundingBox) const
{
	// this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
	(void)outputBoundingBox;
}
