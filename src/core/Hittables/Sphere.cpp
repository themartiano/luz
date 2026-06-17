#include "Hittables/Sphere.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Materials/Lambertian.hpp"
#include "Sampler.hpp"
#include <algorithm>
#include <cmath>

namespace
{
	void	setLatLongUV(const Vector3& outwardNormal, HitRecord& hitRecord)
	{
		const double phi = std::atan2(outwardNormal.getZ(), outwardNormal.getX());
		const double theta = std::acos(std::clamp(outwardNormal.getY(), -1.0, 1.0));

		hitRecord.u = 0.5 + phi / (2.0 * D_PI);
		hitRecord.v = 1.0 - theta / D_PI;
	}

	void	setCubeCrossUV(const Vector3& outwardNormal, HitRecord& hitRecord)
	{
		const double x = outwardNormal.getX();
		const double y = outwardNormal.getY();
		const double z = outwardNormal.getZ();
		const double ax = std::abs(x);
		const double ay = std::abs(y);
		const double az = std::abs(z);
		int column = 0;
		int row = 1;
		double s = 0.5;
		double t = 0.5;

		if (ax >= ay && ax >= az)
		{
			if (x >= 0.0)
			{
				column = 2;
				row = 1;
				s = 0.5 - z / (2.0 * ax);
			}
			else
			{
				column = 0;
				row = 1;
				s = 0.5 + z / (2.0 * ax);
			}
			t = 0.5 + y / (2.0 * ax);
		}
		else if (ay >= ax && ay >= az)
		{
			if (y >= 0.0)
			{
				column = 2;
				row = 2;
				s = 0.5 - z / (2.0 * ay);
			}
			else
			{
				column = 0;
				row = 0;
				s = 0.5 + z / (2.0 * ay);
			}
			t = 0.5 - x / (2.0 * ay);
		}
		else
		{
			if (z >= 0.0)
			{
				column = 1;
				row = 1;
				s = 0.5 + x / (2.0 * az);
			}
			else
			{
				column = 3;
				row = 1;
				s = 0.5 - x / (2.0 * az);
			}
			t = 0.5 + y / (2.0 * az);
		}

		// Match the slight atlas-edge inset used by exported cube-sphere UVs.
		const double faceInset = 0.02;
		s = faceInset + std::clamp(s, 0.0, 1.0) * (1.0 - 2.0 * faceInset);
		t = faceInset + std::clamp(t, 0.0, 1.0) * (1.0 - 2.0 * faceInset);
		hitRecord.u = (static_cast<double>(column) + s) / 4.0;
		hitRecord.v = (static_cast<double>(row) + t) / 3.0;
	}

	void	setSphereUV(const Vector3& outwardNormal, SphereUVProjection uvProjection, HitRecord& hitRecord)
	{
		if (uvProjection == SphereUVProjection::CubeCross)
		{
			setCubeCrossUV(outwardNormal, hitRecord);
			return;
		}
		setLatLongUV(outwardNormal, hitRecord);
	}
}

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
	this->_uvProjection = SphereUVProjection::LatLong;
}

// Constructs the Sphere with custom values
Sphere::Sphere(Vector3 position, double radius, std::shared_ptr<Material> material, SphereUVProjection uvProjection)
{
	this->_position = position;
	this->_radius = radius;
	this->_material = material;
	this->_visible = true;
	this->_uvProjection = uvProjection;
}

Sphere::Sphere(Vector3 position, double radius, std::shared_ptr<Material> material, bool visible, SphereUVProjection uvProjection)
{
	this->_position = position;
	this->_radius = radius;
	this->_material = material;
	this->_visible = visible;
	this->_uvProjection = uvProjection;
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

SphereUVProjection	Sphere::getUVProjection(void) const
{
	return (this->_uvProjection);
}

void	Sphere::setUVProjection(SphereUVProjection uvProjection)
{
	this->_uvProjection = uvProjection;
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
	const Vector3 outwardNormal = Utilities::normalize((hitRecord.position - this->_position) / this->_radius);
	setSphereUV(outwardNormal, this->_uvProjection, hitRecord);
	hitRecord.setFaceNormal(ray, outwardNormal);
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

bool	Sphere::hitInterval(Ray& ray, double t_min, double t_max, double& t0, double& t1) const
{
	Vector3 oc = ray.getOrigin() - this->_position;
	double a = Utilities::dot(ray.getDirection(), ray.getDirection());
	double b = Utilities::dot(oc, ray.getDirection());
	double c = Utilities::dot(oc, oc) - (this->_radius * this->_radius);
	double discriminant = (b * b) - (a * c);

	if (a <= 0.0 || discriminant < 0.0)
	{
		return (false);
	}

	const double sqrtd = sqrt(discriminant);
	double root0 = (-b - sqrtd) / a;
	double root1 = (-b + sqrtd) / a;
	if (root0 > root1)
	{
		std::swap(root0, root1);
	}

	if (root0 < t_min)
	{
		root0 = t_min;
	}
	if (root1 > t_max)
	{
		root1 = t_max;
	}
	if (root0 >= root1)
	{
		return (false);
	}

	t0 = root0;
	t1 = root1;
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
