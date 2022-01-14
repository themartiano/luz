#include "Forms/Sphere.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
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
Sphere::Sphere(Vector3 position, double radius, Material material)
{
	this->_position = position;
	this->_radius = radius;
	this->_material = material;
}

// Returns the Sphere's position
Vector3	Sphere::getPosition(void) const
{
	return (this->_position);
}

// Sets the Sphere's position
void	Sphere::setPosition(Vector3 position)
{
	this->_position = position;
}

// Returns the Sphere's radius
double	Sphere::getRadius(void) const
{
	return (this->_radius);
}

// Sets the Sphere's radius
void	Sphere::setRadius(double radius)
{
	this->_radius = radius;
}

// Returns the Sphere's material
Material	Sphere::getMaterial(void) const
{
	return (this->_material);
}

// Sets the Sphere's material
void	Sphere::setMaterial(Material material)
{
	this->_material = material;
}

// Calculates if the Sphere is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Sphere::hit(Ray& ray, double t_max) const
{
	Vector3 oc = ray.getOrigin() - this->_position;
	double a = Utilities::dot(ray.getDirection(), ray.getDirection());
	double b = Utilities::dot(oc, ray.getDirection());
	double c = Utilities::dot(oc, oc) - (this->_radius * this->_radius);
	double discriminant = (b * b) - (a * c);

	if (discriminant < 0.0)
	{
		return (false);
	}

	double sqrtd = sqrt(discriminant);
	double root = (-b - sqrtd) / a;
	if (root < T_MIN || root > t_max)
	{
		root = (-b + sqrtd) / a;
		if (root < T_MIN || root > t_max)
		{
			return (false);
		}
	}

	ray.hitRecord.t0 = root;
	ray.hitRecord.position = ray.pointAtRay(root);
	ray.hitRecord.normal = Utilities::normalize((ray.hitRecord.position - this->_position) / this->_radius);
	ray.hitRecord.material = this->_material;

	return (true);
}

// Creates an AABB / bounding box for this Sphere
bool	Sphere::createBoundingBox(AABB& outputBoundingBox) const
{
	outputBoundingBox = AABB(
		this->_position - Vector3(this->_radius, this->_radius, this->_radius),
		this->_position + Vector3(this->_radius, this->_radius, this->_radius)
	);

	return (true);
}

double  Sphere::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	Ray ray(origin, vec);
	if (!this->hit(ray, T_MAX))
	{
		return (0.0);
	}

	double cosThetaMax = sqrt(1.0 - this->_radius * this->_radius / Utilities::vectorLengthSquared(this->_position - origin));
	double solidAngle = 2.0 * D_PI * (1.0 - cosThetaMax);

	return (1.0 / solidAngle);
}

Vector3	Sphere::random(const Vector3& origin) const
{
	Vector3	direction = this->_position - origin;
	double distanceSquared = Utilities::vectorLengthSquared(direction);

	ONB uvw(direction);

	return (uvw.local(randomToSphere(distanceSquared)));
}

Vector3 Sphere::randomToSphere(double distanceSquared) const
{
	double rand1 = Utilities::randomDouble();
	double rand2 = Utilities::randomDouble();
	double z = 1.0 + rand2 * (sqrt(1.0 - this->_radius * this->_radius / distanceSquared) - 1.0);

	double phi = 2.0 * D_PI * rand1;
	double x = cos(phi) * sqrt(1.0 - z * z);
	double y = sin(phi) * sqrt(1.0 - z * z);

	return (Vector3(x, y, z));
}
