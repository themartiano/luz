#include "Hittables/Plane.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Materials/Lambertian.hpp"

/*
	Constructors
*/

// Constructs the Plane with default values
Plane::Plane(void)
{
	this->_y = 0.0;
	this->_orientation = Vector3(0.0, -1.0, 0.0);
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
}

// Constructs the Plane with custom values
Plane::Plane(double y, Vector3 orientation, std::shared_ptr<Material> material)
{
	this->_y = y;
	this->_orientation = orientation;
	this->_material = material;
}

// Sets the Plane's Y value
void	Plane::setY(double y)
{
	this->_y = y;
}

// Sets the Plane's Orientation
void	Plane::setOrientation(Vector3 orientation)
{
	this->_orientation = orientation;
}

// Returns the Plane's material
Material*	Plane::getMaterial(void) const
{
	return (this->_material.get());
}

// Sets the Plane's Material
void	Plane::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Calculates if the Plane is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Plane::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3 position(0.0, this->_y, 0.0);

	double d = Utilities::dot(ray.getDirection(), this->_orientation);
	if (d == 0.0)
	{
		return (false);
	}

	double t = Utilities::dot(position - ray.getOrigin(), this->_orientation) / d;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	hitRecord.t0 = t;
	hitRecord.setFaceNormal(ray, this->_orientation);
	hitRecord.material = this->_material.get();
	hitRecord.position = ray.pointAtRay(t);

	return (true);
}

bool	Plane::hitAny(Ray& ray, double t_min, double t_max) const
{
	Vector3 position(0.0, this->_y, 0.0);

	double d = Utilities::dot(ray.getDirection(), this->_orientation);
	if (d == 0.0)
	{
		return (false);
	}

	double t = Utilities::dot(position - ray.getOrigin(), this->_orientation) / d;
	return (t >= t_min && t <= t_max);
}

// Won't create an AABB / bounding box because planes can't have them since they're infinite. Returns false
bool	Plane::createBoundingBox(AABB& boundingBox) const
{
	return (false);
	(void)boundingBox;
}
