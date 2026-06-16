#include "Hittables/Sphere.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Materials/Lambertian.hpp"
#include "Sampler.hpp"
#include <algorithm>
#include <cmath>

/*
	Constructors
*/

// Constructs the Sphere with default values
Sphere::Sphere(void)
{
	this->_position = Vector3();
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_radius = 1.0;
	this->_visible = true;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Vector3 position, double radius, std::shared_ptr<Material> material)
{
	this->_position = position;
	this->_radius = radius;
	this->_material = material;
	this->_visible = true;
}

Sphere::Sphere(Vector3 position, double radius, std::shared_ptr<Material> material, bool visible)
{
	this->_position = position;
	this->_radius = radius;
	this->_material = material;
	this->_visible = visible;
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

bool	Sphere::isVisible(void) const
{
	return (this->_visible);
}

void	Sphere::setVisible(bool visible)
{
	this->_visible = visible;
}

// Returns the Sphere's material
Material*	Sphere::getMaterial(void) const
{
	return (this->_material.get());
}

// Sets the Sphere's material
void	Sphere::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Calculates if the Sphere is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Sphere::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	if (!this->_visible)
	{
		return (false);
	}

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
	if (Utilities::dot(n, ray.getDirection()) > 0.0)
	{
		n = n * -1.0;
	}
	hitRecord.normal = n;
	hitRecord.material = this->_material.get();

	return (true);
}

bool	Sphere::hitAny(Ray& ray, double t_min, double t_max) const
{
	if (!this->_visible)
	{
		return (false);
	}

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
	if (root >= t_min && root <= t_max)
	{
		return (true);
	}
	root = (-b + sqrtd) / a;
	return (root >= t_min && root <= t_max);
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
	HitRecord	hitRecord;
	if (!this->hit(ray, hitRecord, T_MIN, T_MAX))
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
	const Sampler::Sample2D sample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	const float radiusRatioSquared = static_cast<float>(this->_radius * this->_radius / distanceSquared);
	const float z = 1.0f + sample.y * (std::sqrt(std::max(0.0f, 1.0f - radiusRatioSquared)) - 1.0f);

	const float phi = static_cast<float>(2.0 * D_PI) * sample.x;
	const float radius = std::sqrt(std::max(0.0f, 1.0f - z * z));
	const float x = std::cos(phi) * radius;
	const float y = std::sin(phi) * radius;

	return (Vector3(x, y, z));
}

bool	Sphere::sampleLight(const Vector3& origin, HittableLightSample& sample) const
{
	sample = HittableLightSample();

	const double distanceSquared = Utilities::vectorLengthSquared(this->_position - origin);
	if (
		this->_radius <= 0.0
		|| !std::isfinite(distanceSquared)
		|| distanceSquared <= this->_radius * this->_radius
	)
	{
		return (false);
	}

	sample.direction = this->random(origin);
	const double directionLengthSquared = Utilities::vectorLengthSquared(sample.direction);
	if (!std::isfinite(directionLengthSquared) || directionLengthSquared <= 0.0)
	{
		return (false);
	}
	sample.direction /= std::sqrt(directionLengthSquared);

	Vector3 oc = origin - this->_position;
	const double a = Utilities::vectorLengthSquared(sample.direction);
	const double b = Utilities::dot(oc, sample.direction);
	const double c = Utilities::vectorLengthSquared(oc) - (this->_radius * this->_radius);
	const double discriminant = (b * b) - (a * c);
	if (discriminant < 0.0)
	{
		return (false);
	}

	const double sqrtd = std::sqrt(discriminant);
	double root = (-b - sqrtd) / a;
	if (root < T_MIN || root > T_MAX)
	{
		root = (-b + sqrtd) / a;
	}
	if (root < T_MIN || root > T_MAX)
	{
		return (false);
	}

	const double cosThetaMax = std::sqrt(1.0 - this->_radius * this->_radius / distanceSquared);
	const double solidAngle = 2.0 * D_PI * (1.0 - cosThetaMax);
	if (solidAngle <= 0.0 || !std::isfinite(solidAngle))
	{
		return (false);
	}

	sample.pdf = 1.0 / solidAngle;
	sample.tMax = root;
	sample.material = this->_material.get();
	sample.valid = std::isfinite(sample.pdf) && sample.pdf > 0.0 && sample.tMax > T_MIN;
	return (sample.valid);
}

double	Sphere::lightSelectionWeight(void) const
{
	if (!this->_material)
	{
		return (0.0);
	}

	const double area = 4.0 * D_PI * this->_radius * this->_radius;
	const double luminance = Utilities::luminance(this->_material->emitted());
	if (area <= 0.0 || !std::isfinite(luminance) || luminance <= 0.0)
	{
		return (0.0);
	}
	return (area * luminance);
}
