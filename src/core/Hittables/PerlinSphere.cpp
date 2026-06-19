#include "Hittables/PerlinSphere.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Materials/Lambertian.hpp"
#include "Sampler.hpp"
#include <algorithm>
#include <cmath>

namespace
{
	bool	zeroDirection(const Vector3& direction)
	{
		return (
			direction.getX() == 0.0
			&& direction.getY() == 0.0
			&& direction.getZ() == 0.0
		);
	}
}

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
Material*	PerlinSphere::getMaterial(void) const
{
	return (this->_material.get());
}

// Sets the PerlinSphere's material
void	PerlinSphere::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Calculates if the PerlinSphere is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	PerlinSphere::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	const Vector3& direction = ray.getDirection();
	if (zeroDirection(direction))
	{
		return (false);
	}

	Vector3 oc = ray.getOrigin() - this->_position;
	double b = Utilities::dot(oc, direction);
	double c = Utilities::vectorLengthSquared(oc) - (this->_radius * this->_radius);
	double discriminant = (b * b) - c;

	if (discriminant < 0.0)
	{
		return (false);
	}

	double sqrtd = sqrt(discriminant);
	double root = -b - sqrtd;
	if (root < t_min || root > t_max)
	{
		root = -b + sqrtd;
		if (root < t_min || root > t_max)
		{
			return (false);
		}
	}

	hitRecord.t0 = root;
	hitRecord.position = ray.pointAtRay(root);

	const Vector3 outwardNormal = (hitRecord.position - this->_position) / this->_radius;
	hitRecord.setFaceNormal(ray, outwardNormal);
	hitRecord.material = this->_material.get();

	Vector3 hitPosForNoise = hitRecord.position + this->_radius;
	hitRecord.material->setColor(
		Color(1.0, 1.0, 1.0) * 0.5 * (1.0 + sin(hitPosForNoise.getZ() + 10.0 * this->_perlin.noise(hitPosForNoise * 10)))
	);

	return (true);
}

bool	PerlinSphere::hitAny(Ray& ray, double t_min, double t_max) const
{
	const Vector3& direction = ray.getDirection();
	if (zeroDirection(direction))
	{
		return (false);
	}

	Vector3 oc = ray.getOrigin() - this->_position;
	double b = Utilities::dot(oc, direction);
	double c = Utilities::vectorLengthSquared(oc) - (this->_radius * this->_radius);
	double discriminant = (b * b) - c;

	if (discriminant < 0.0)
	{
		return (false);
	}

	double sqrtd = sqrt(discriminant);
	double root = -b - sqrtd;
	if (root >= t_min && root <= t_max)
	{
		return (true);
	}
	root = -b + sqrtd;
	return (root >= t_min && root <= t_max);
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
	const float radiusRatioSquared = static_cast<float>(this->_radius * this->_radius / distanceSquared);
	const float z = 1.0f + sample.y * (std::sqrt(std::max(0.0f, 1.0f - radiusRatioSquared)) - 1.0f);

	const float phi = static_cast<float>(2.0 * D_PI) * sample.x;
	const float radius = std::sqrt(std::max(0.0f, 1.0f - z * z));
	const float x = std::cos(phi) * radius;
	const float y = std::sin(phi) * radius;

	return (Vector3(x, y, z));
}

bool	PerlinSphere::sampleLight(const Vector3& origin, HittableLightSample& sample) const
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

bool	PerlinSphere::sampleEmission(HittableEmissionSample& sample) const
{
	sample = HittableEmissionSample();
	if (!this->_material || this->_radius <= 0.0)
	{
		return (false);
	}

	const Color emitted = this->_material->emitted();
	if (Utilities::luminance(emitted) <= 0.0)
	{
		return (false);
	}

	const Vector3 outwardNormal = Utilities::normalize(Sampler::sphereDirection(Sampler::DIM_LIGHT_SURFACE_POINT));
	const ONB basis(outwardNormal);

	sample.position = this->_position + outwardNormal * this->_radius;
	sample.normal = outwardNormal;
	sample.direction = Utilities::normalize(basis.local(Sampler::cosineHemisphere(Sampler::DIM_LIGHT_EMISSION_DIRECTION)));
	sample.emitted = emitted;
	sample.powerScale = D_PI * 4.0 * D_PI * this->_radius * this->_radius;
	sample.valid = Utilities::vectorLengthSquared(sample.direction) > 0.0;
	return (sample.valid);
}

double	PerlinSphere::lightSelectionWeight(void) const
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

void	PerlinSphere::calculateSphereUV(const Vector3& point, double& u, double& v) const
{
	double theta = acos(-point.getY());
	double phi = atan2(-point.getZ(), point.getX()) + D_PI;

	u = phi / (2.0 * D_PI);
	v = theta / D_PI;
}
