#include "Forms/Sphere.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Sphere with default values
Sphere::Sphere(void)
{
	this->_position = Vector3();
	this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
	this->_radius = 1.0;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Vector3 position, Material material, double radius)
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
double	Sphere::getRadius(void) const
{
	return (this->_radius);
}

// Calculates if the Sphere is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Sphere::hit(Ray& ray, double t_max) const
{
	Vector3 oc = ray.getOrigin() - this->_position;
	double a = dot(ray.getDirection(), ray.getDirection());
	double b = dot(oc, ray.getDirection());
	double c = dot(oc, oc) - (this->_radius * this->_radius);
	double discriminant = (b * b) - (a * c);

	if (discriminant > 0.0)
    {
        double temp1 = (-b - sqrt((b * b) - (a * c))) / a;
        double temp2 = (-b + sqrt((b * b) - (a * c))) / a;

        if (temp1 < t_max && temp1 > T_MIN)
        {
            ray.hitRecord.t0 = temp1;
            ray.hitRecord.t1 = temp2;
        }
        else if (temp2 < t_max && temp2 > T_MIN)
        {
            ray.hitRecord.t0 = temp2;
            ray.hitRecord.t1 = temp1;
        }

        ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t0);
        ray.hitRecord.normal = (ray.hitRecord.position - this->_position) / this->_radius;
        ray.hitRecord.material = this->_material;

        return (true);
    }

    return (false);
}

// Creates an AABB / bounding box for this Sphere
bool    Sphere::createBoundingBox(AABB& outputBoundingBox) const
{
	outputBoundingBox = AABB(
        this->_position - Vector3(this->_radius, this->_radius, this->_radius),
        this->_position + Vector3(this->_radius, this->_radius, this->_radius));

    return (true);
}
