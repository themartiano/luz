#include "Forms/Sphere.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Sphere with default values
Sphere::Sphere(void)
{
	this->_position = Vector3();
	this->_material = Material(Color(0.49f, 0.49f, 0.49f), 1.0f, 0.0f, 0.5f, 0.0f, false);
	this->_radius = 1.0f;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Vector3 position, Material material, float radius)
{
	this->_position = position;
	this->_material = material;
	this->_radius = radius;
}

// Returns the Sphere's position
Vector3	Sphere::getPosition(void) const
{
	return (this->_position);
}

// Returns the Sphere's radius
float	Sphere::getRadius(void) const
{
	return (this->_radius);
}

// Calculates if 'sphere' is hit by 'ray', is closer than 't_max' and farther than T_MIN
virtual bool    Sphere::hit(Ray& ray, float t_max) const override
{
	Vector3 oc = ray.getOrigin() - sphere.getPosition();
	float a = dot(ray.getDirection(), ray.getDirection());
	float b = dot(oc, ray.getDirection());
	float c = dot(oc, oc) - (sphere.getRadius() * sphere.getRadius());
	float discriminant = (b * b) - (a * c);

	if (discriminant > 0.0f)
    {
        float temp = (-b - sqrt((b * b) - (a * c))) / a;
        if (temp < t_max && temp > T_MIN)
        {
            ray.hitRecord.t = temp;
            ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t);
            ray.hitRecord.normal = (ray.hitRecord.position - sphere.getPosition()) / sphere.getRadius();
            return (true);
        }

        temp = (-b + sqrt((b * b) - (a * c))) / a;
        if (temp < t_max && temp > T_MIN)
        {
            ray.hitRecord.t = temp;
            ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t);
            ray.hitRecord.normal = (ray.hitRecord.position - sphere.getPosition()) / sphere.getRadius();
            return (true);
        }
    }

    return (false);
}

// Creates an AABB / bounding box for this Sphere
virtual AABB    Sphere::createBoundingBox(void) const override
{
	return (AABB(this->_position - Vector3(this->_radius, this->_radius, this->_radius), this->_position + Vector3(this->_radius, this->_radius, this->_radius)));
}

// Returns the Sphere's material
virtual Material	Sphere::getMaterial(void) const override
{
	return (this->_material);
}
