#include "Hittables/Triangle.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Materials/Lambertian.hpp"
#include "ONB.hpp"
#include "Sampler.hpp"
#include <algorithm>
#include <cmath>

namespace
{
	const double	TRIANGLE_DETERMINANT_EPSILON = 1e-12;
	const double	TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED = 1e-24;

	Vector3	normalizedOrFallback(Vector3 normal, Vector3 fallback)
	{
		if (Utilities::vectorLengthSquared(normal) <= TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED)
		{
			return (fallback);
		}
		return (Utilities::normalize(normal));
	}

}

/*
	Constructors
*/

// Constructs the Triangle with default values
Triangle::Triangle(void)
{
	this->_vertex0 = Vector3(0.0, 1.0, 0.0);
	this->_vertex1 = Vector3(-1.0, 0.0, 0.0);
	this->_vertex2 = Vector3(1.0, 0.0, 0.0);
	this->_normal0 = Vector3();
	this->_normal1 = Vector3();
	this->_normal2 = Vector3();
	this->_uv0 = Vector3();
	this->_uv1 = Vector3();
	this->_uv2 = Vector3();
	this->_hasVertexNormals = false;
	this->_hasTextureCoordinates = false;
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_updateCache();
}

// Constructs the Triangle with custom values
Triangle::Triangle(Vector3 vertex0, Vector3 vertex1, Vector3 vertex2, std::shared_ptr<Material> material)
{
	this->_vertex0 = vertex0;
	this->_vertex1 = vertex1;
	this->_vertex2 = vertex2;
	this->_normal0 = Vector3();
	this->_normal1 = Vector3();
	this->_normal2 = Vector3();
	this->_uv0 = Vector3();
	this->_uv1 = Vector3();
	this->_uv2 = Vector3();
	this->_hasVertexNormals = false;
	this->_hasTextureCoordinates = false;
	this->_material = material;
	this->_updateCache();
}

Triangle::Triangle(
	Vector3 vertex0,
	Vector3 vertex1,
	Vector3 vertex2,
	Vector3 normal0,
	Vector3 normal1,
	Vector3 normal2,
	std::shared_ptr<Material> material
)
{
	this->_vertex0 = vertex0;
	this->_vertex1 = vertex1;
	this->_vertex2 = vertex2;
	this->_normal0 = normal0;
	this->_normal1 = normal1;
	this->_normal2 = normal2;
	this->_uv0 = Vector3();
	this->_uv1 = Vector3();
	this->_uv2 = Vector3();
	this->_hasVertexNormals =
		Utilities::vectorLengthSquared(normal0) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED
		&& Utilities::vectorLengthSquared(normal1) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED
		&& Utilities::vectorLengthSquared(normal2) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED;
	this->_hasTextureCoordinates = false;
	this->_material = material;
	this->_updateCache();
}

void	Triangle::_updateCache(void)
{
	this->_edge1 = this->_vertex1 - this->_vertex0;
	this->_edge2 = this->_vertex2 - this->_vertex0;

	const Vector3	geometricNormal = Utilities::cross(this->_edge1, this->_edge2);
	const double	normalLengthSquared = Utilities::vectorLengthSquared(geometricNormal);
	this->_isDegenerate = normalLengthSquared <= TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED;
	if (this->_isDegenerate)
	{
		this->_faceNormal = Vector3();
		this->_area = 0.0;
	}
	else
	{
		const double normalLength = std::sqrt(normalLengthSquared);
		this->_faceNormal = geometricNormal / normalLength;
		this->_area = normalLength / 2.0;
	}

	Vector3 minimum(
		std::min(this->_vertex0.getX(), std::min(this->_vertex1.getX(), this->_vertex2.getX())) - T_MIN,
		std::min(this->_vertex0.getY(), std::min(this->_vertex1.getY(), this->_vertex2.getY())) - T_MIN,
		std::min(this->_vertex0.getZ(), std::min(this->_vertex1.getZ(), this->_vertex2.getZ())) - T_MIN
	);
	Vector3 maximum(
		std::max(this->_vertex0.getX(), std::max(this->_vertex1.getX(), this->_vertex2.getX())) + T_MIN,
		std::max(this->_vertex0.getY(), std::max(this->_vertex1.getY(), this->_vertex2.getY())) + T_MIN,
		std::max(this->_vertex0.getZ(), std::max(this->_vertex1.getZ(), this->_vertex2.getZ())) + T_MIN
	);

	this->_boundingBox = AABB(minimum, maximum);
}

// Sets the Triangle's Vertex-0
void	Triangle::setVertex0(Vector3 vertex0)
{
	this->_vertex0 = vertex0;
	this->_updateCache();
}

// Sets the Triangle's Vertex-1
void	Triangle::setVertex1(Vector3 vertex1)
{
	this->_vertex1 = vertex1;
	this->_updateCache();
}

// Sets the Triangle's Vertex-2
void	Triangle::setVertex2(Vector3 vertex2)
{
	this->_vertex2 = vertex2;
	this->_updateCache();
}

void	Triangle::setVertexNormals(Vector3 normal0, Vector3 normal1, Vector3 normal2)
{
	this->_normal0 = normal0;
	this->_normal1 = normal1;
	this->_normal2 = normal2;
	this->_hasVertexNormals =
		Utilities::vectorLengthSquared(normal0) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED
		&& Utilities::vectorLengthSquared(normal1) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED
		&& Utilities::vectorLengthSquared(normal2) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED;
}

void	Triangle::setTextureCoordinates(Vector3 uv0, Vector3 uv1, Vector3 uv2)
{
	this->_uv0 = uv0;
	this->_uv1 = uv1;
	this->_uv2 = uv2;
	this->_hasTextureCoordinates = true;
}

// Returns the Triangle's material
Material*	Triangle::getMaterial(void) const
{
	return (this->_material.get());
}

// Sets the Triangle's Material
void	Triangle::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Calculates if the Triangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Triangle::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	if (this->_isDegenerate)
	{
		return (false);
	}

	const Vector3& rayDirection = ray.getDirection();
	const Vector3& rayOrigin = ray.getOrigin();
	Vector3 p = Utilities::cross(rayDirection, this->_edge2);
	double  det = Utilities::dot(this->_edge1, p);

	// det is an area-scaled value, not a ray distance, so it needs its own epsilon.
	if (fabs(det) < TRIANGLE_DETERMINANT_EPSILON)
	{
		return (false);
	}

	const double invDet = 1.0 / det;
	Vector3 tVec = rayOrigin - this->_vertex0;
	double u = Utilities::dot(tVec, p) * invDet;
	if (u < 0.0 || u > 1.0)
	{
		return (false);
	}

	Vector3 q = Utilities::cross(tVec, this->_edge1);
	double v = Utilities::dot(rayDirection, q) * invDet;
	if (v < 0.0 || u + v > 1.0)
	{
		return (false);
	}

	double t = Utilities::dot(this->_edge2, q) * invDet;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	hitRecord.t0 = t;
	if (this->_hasVertexNormals)
	{
		const Vector3 interpolatedNormal =
			(this->_normal0 * (1.0 - u - v))
			+ (this->_normal1 * u)
			+ (this->_normal2 * v);
		hitRecord.normal = normalizedOrFallback(interpolatedNormal, this->_faceNormal);
	}
	else
	{
		hitRecord.normal = this->_faceNormal;
	}
	if (this->_hasTextureCoordinates)
	{
		const Vector3 interpolatedUV =
			(this->_uv0 * (1.0 - u - v))
			+ (this->_uv1 * u)
			+ (this->_uv2 * v);
		hitRecord.u = interpolatedUV.getX();
		hitRecord.v = interpolatedUV.getY();
	}
	else
	{
		hitRecord.u = 0.0;
		hitRecord.v = 0.0;
	}
	hitRecord.setFaceNormal(ray, hitRecord.normal);
	hitRecord.material = this->_material.get();
	hitRecord.position = ray.pointAtRay(hitRecord.t0);

	return (true);
}

bool	Triangle::hitAny(Ray& ray, double t_min, double t_max) const
{
	if (this->_isDegenerate)
	{
		return (false);
	}

	const Vector3& rayDirection = ray.getDirection();
	const Vector3& rayOrigin = ray.getOrigin();
	Vector3 p = Utilities::cross(rayDirection, this->_edge2);
	double  det = Utilities::dot(this->_edge1, p);
	if (fabs(det) < TRIANGLE_DETERMINANT_EPSILON)
	{
		return (false);
	}

	const double invDet = 1.0 / det;
	Vector3 tVec = rayOrigin - this->_vertex0;
	double u = Utilities::dot(tVec, p) * invDet;
	if (u < 0.0 || u > 1.0)
	{
		return (false);
	}

	Vector3 q = Utilities::cross(tVec, this->_edge1);
	double v = Utilities::dot(rayDirection, q) * invDet;
	if (v < 0.0 || u + v > 1.0)
	{
		return (false);
	}

	double t = Utilities::dot(this->_edge2, q) * invDet;
	return (t >= t_min && t <= t_max);
}

// Creates an AABB / bounding box for this Triangle
bool	Triangle::createBoundingBox(AABB& outputBoundingBox) const
{
	outputBoundingBox = this->_boundingBox;

	return (true);
}

double	Triangle::area(void) const
{
	return (this->_area);
}

double	Triangle::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	Ray ray(origin, vec);
	HitRecord hitRecord;

	if (!this->hit(ray, hitRecord, T_MIN, T_MAX))
	{
		return (0.0);
	}

	const double triangleArea = this->area();
	if (triangleArea <= 0.0)
	{
		return (0.0);
	}

	HitRecord geometricHitRecord;
	geometricHitRecord.setFaceNormal(ray, this->_faceNormal);

	const double distanceSquared = hitRecord.t0 * hitRecord.t0 * Utilities::vectorLengthSquared(vec);
	const double cosine = std::fabs(Utilities::dot(vec, geometricHitRecord.normal) / Utilities::vectorLength(vec));
	if (cosine <= 0.0)
	{
		return (0.0);
	}

	return (distanceSquared / (cosine * triangleArea));
}

Vector3	Triangle::random(const Vector3& origin) const
{
	Sampler::Sample2D sample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	double u = sample.x;
	double v = sample.y;

	if (u + v > 1.0)
	{
		u = 1.0 - u;
		v = 1.0 - v;
	}

	const Vector3 randomPoint = this->_vertex0
		+ ((this->_vertex1 - this->_vertex0) * u)
		+ ((this->_vertex2 - this->_vertex0) * v);

	return (Utilities::normalize(randomPoint - origin));
}

bool	Triangle::sampleLight(const Vector3& origin, HittableLightSample& sample) const
{
	sample = HittableLightSample();
	if (this->_isDegenerate || this->_area <= 0.0)
	{
		return (false);
	}

	Sampler::Sample2D surfaceSample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	double u = surfaceSample.x;
	double v = surfaceSample.y;
	if (u + v > 1.0)
	{
		u = 1.0 - u;
		v = 1.0 - v;
	}

	const Vector3 randomPoint = this->_vertex0
		+ ((this->_vertex1 - this->_vertex0) * u)
		+ ((this->_vertex2 - this->_vertex0) * v);
	const Vector3 direction = randomPoint - origin;
	const double distanceSquared = Utilities::vectorLengthSquared(direction);
	if (!std::isfinite(distanceSquared) || distanceSquared <= 0.0)
	{
		return (false);
	}

	const double distance = std::sqrt(distanceSquared);
	sample.direction = direction / distance;

	const double cosine = std::fabs(Utilities::dot(sample.direction, this->_faceNormal));
	if (cosine <= 0.0)
	{
		return (false);
	}

	sample.pdf = distanceSquared / (cosine * this->_area);
	sample.tMax = distance;
	sample.material = this->_material.get();
	sample.valid = std::isfinite(sample.pdf) && sample.pdf > 0.0;
	return (sample.valid);
}

bool	Triangle::sampleEmission(HittableEmissionSample& sample) const
{
	sample = HittableEmissionSample();
	if (!this->_material || this->_isDegenerate || this->_area <= 0.0)
	{
		return (false);
	}

	const Color emitted = this->_material->emitted();
	if (Utilities::luminance(emitted) <= 0.0)
	{
		return (false);
	}

	Sampler::Sample2D surfaceSample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	double u = surfaceSample.x;
	double v = surfaceSample.y;
	if (u + v > 1.0)
	{
		u = 1.0 - u;
		v = 1.0 - v;
	}

	Vector3 normal = this->_faceNormal;
	if (Sampler::sample1D(Sampler::DIM_LIGHT_EMISSION_SIDE) < 0.5)
	{
		normal = normal * -1.0;
	}
	const ONB basis(normal);

	sample.position = this->_vertex0
		+ ((this->_vertex1 - this->_vertex0) * u)
		+ ((this->_vertex2 - this->_vertex0) * v);
	sample.normal = normal;
	sample.direction = Utilities::normalize(basis.local(Sampler::cosineHemisphere(Sampler::DIM_LIGHT_EMISSION_DIRECTION)));
	sample.emitted = emitted;
	sample.powerScale = 2.0 * D_PI * this->_area;
	sample.valid = Utilities::vectorLengthSquared(sample.direction) > 0.0;
	return (sample.valid);
}

double	Triangle::lightSelectionWeight(void) const
{
	if (!this->_material)
	{
		return (0.0);
	}

	const double luminance = Utilities::luminance(this->_material->emitted());
	if (this->_area <= 0.0 || !std::isfinite(luminance) || luminance <= 0.0)
	{
		return (0.0);
	}
	return (this->_area * luminance);
}
