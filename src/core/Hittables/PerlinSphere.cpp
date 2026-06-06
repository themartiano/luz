#include "Hittables/PerlinSphere.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Materials/Lambertian.hpp"
#include "Sampler.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the PerlinSphere with default values
PerlinSphere::PerlinSphere(void)
{
	this->_position = Vector3();
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_radius = 1.0;
	this->_seed = 42;

	this->_perlin = Perlin(this->_seed);
}

// Constructs the PerlinSphere with custom values
PerlinSphere::PerlinSphere(Vector3 position, double radius, std::shared_ptr<Material> material, unsigned int seed)
{
	this->_position = position;
	this->_radius = radius;
	this->_material = material;
	this->_seed = seed;

	this->_perlin = Perlin(this->_seed);
}

// Returns the PerlinSphere's position
Vector3	PerlinSphere::getPosition(void) const
{
	return (this->_position);
}

// Sets the PerlinSphere's position
void	PerlinSphere::setPosition(Vector3 position)
{
	this->_position = position;
}

// Returns the PerlinSphere's radius
double	PerlinSphere::getRadius(void) const
{
	return (this->_radius);
}

// Sets the PerlinSphere's radius
void	PerlinSphere::setRadius(double radius)
{
	this->_radius = radius;
}

// Returns the PerlinSphere's material
std::shared_ptr<Material>	PerlinSphere::getMaterial(void) const
{
	return (this->_material);
}

// Sets the PerlinSphere's material
void	PerlinSphere::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Calculates if the PerlinSphere is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	PerlinSphere::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3 oc = ray.getOrigin() - this->_position;
	double a = Utilities::vectorLengthSquared(ray.getDirection());
	double b = Utilities::dot(oc, ray.getDirection());
	double c = Utilities::vectorLengthSquared(oc) - (this->_radius * this->_radius);
	double discriminant = (b * b) - (a * c);

	if (discriminant < 0.0)
	{
		return (false);
	}

	double sqrtd = sqrt(discriminant);
	double root = (-b - sqrtd) / a;
	if (root < t_min || root > t_max)
	{
		root = (-b + sqrtd) / a;
		if (root < t_min || root > t_max)
		{
			return (false);
		}
	}

	hitRecord.t0 = root;
	hitRecord.position = ray.pointAtRay(root);

	Vector3 n = Utilities::normalize((hitRecord.position - this->_position) / this->_radius);
	// if (Utilities::dot(n, ray.getDirection()) > 0.0)
	// {
	// 	n = n * -1.0;
	// }
	hitRecord.normal = n;
	hitRecord.material = this->_material;

	Vector3 hitPosForNoise = hitRecord.position + this->_radius;
	hitRecord.material->setColor(
		Color(1.0, 1.0, 1.0) * 0.5 * (1.0 + sin(hitPosForNoise.getZ() + 10.0 * this->_perlin.noise(hitPosForNoise * 10)))
	);

	return (true);
}

// Creates an AABB / bounding box for this PerlinSphere
bool	PerlinSphere::createBoundingBox(AABB& outputBoundingBox) const
{
	outputBoundingBox = AABB(
		this->_position - Vector3(this->_radius, this->_radius, this->_radius),
		this->_position + Vector3(this->_radius, this->_radius, this->_radius)
	);

	return (true);
}

double  PerlinSphere::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	Ray ray(origin, vec);
	HitRecord	hitRecord;
	if (!this->hit(ray, hitRecord, T_MIN, T_MAX))
	{
		return (0.0);
	}

	double cosThetaMax = sqrt(1.0 - this->_radius * this->_radius / Utilities::vectorLengthSquared(this->_position - origin));
	double solidAngle = 2.0 * D_PI * (1.0 - cosThetaMax);

	return (1.0 / solidAngle);
}

Vector3	PerlinSphere::random(const Vector3& origin) const
{
	Vector3	direction = this->_position - origin;
	double distanceSquared = Utilities::vectorLengthSquared(direction);

	ONB uvw(direction);

	return (uvw.local(randomToSphere(distanceSquared)));
}

Vector3 PerlinSphere::randomToSphere(double distanceSquared) const
{
	const Sampler::Sample2D sample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	double z = 1.0 + sample.y * (sqrt(1.0 - this->_radius * this->_radius / distanceSquared) - 1.0);

	double phi = 2.0 * D_PI * sample.x;
	double x = cos(phi) * sqrt(1.0 - z * z);
	double y = sin(phi) * sqrt(1.0 - z * z);

	return (Vector3(x, y, z));
}

void	PerlinSphere::calculateSphereUV(const Vector3& point, double& u, double& v) const
{
	double theta = acos(-point.getY());
	double phi = atan2(-point.getZ(), point.getX()) + D_PI;

	u = phi / (2.0 * D_PI);
	v = theta / D_PI;
}
