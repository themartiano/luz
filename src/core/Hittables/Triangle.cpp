#include "Hittables/Triangle.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Materials/Lambertian.hpp"
#include "Random.hpp"
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

	Vector3	orientAgainstRay(Vector3 normal, const Ray& ray)
	{
		if (Utilities::dot(normal, ray.getDirection()) > 0.0)
		{
			return (normal * -1.0);
		}
		return (normal);
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
	this->_hasVertexNormals = false;
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
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
	this->_hasVertexNormals = false;
	this->_material = material;
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
	this->_hasVertexNormals =
		Utilities::vectorLengthSquared(normal0) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED
		&& Utilities::vectorLengthSquared(normal1) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED
		&& Utilities::vectorLengthSquared(normal2) > TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED;
	this->_material = material;
}

// Sets the Triangle's Vertex-0
void	Triangle::setVertex0(Vector3 vertex0)
{
	this->_vertex0 = vertex0;
}

// Sets the Triangle's Vertex-1
void	Triangle::setVertex1(Vector3 vertex1)
{
	this->_vertex1 = vertex1;
}

// Sets the Triangle's Vertex-2
void	Triangle::setVertex2(Vector3 vertex2)
{
	this->_vertex2 = vertex2;
}

// Returns the Triangle's material
std::shared_ptr<Material>	Triangle::getMaterial(void) const
{
	return (this->_material);
}

// Sets the Triangle's Material
void	Triangle::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Calculates if the Triangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Triangle::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3 v1 = this->_vertex1 - this->_vertex0;
	Vector3 v2 = this->_vertex2 - this->_vertex0;
	Vector3 n = Utilities::cross(v1, v2);
	const double normalLengthSquared = Utilities::vectorLengthSquared(n);

	if (normalLengthSquared <= TRIANGLE_NORMAL_LENGTH_EPSILON_SQUARED)
	{
		return (false);
	}

	Vector3 p = Utilities::cross(ray.getDirection(), v2);
	double  det = Utilities::dot(v1, p);

	// det is an area-scaled value, not a ray distance, so it needs its own epsilon.
	if (fabs(det) < TRIANGLE_DETERMINANT_EPSILON)
	{
		return (false);
	}

	Vector3 tVec = ray.getOrigin() - this->_vertex0;
	double u = Utilities::dot(tVec, p) / det;
	if (u < 0.0 || u > 1.0)
	{
		return (false);
	}

	Vector3 q = Utilities::cross(tVec, v1);
	double v = Utilities::dot(ray.getDirection(), q) / det;
	if (v < 0.0 || u + v > 1.0)
	{
		return (false);
	}

	double t = Utilities::dot(v2, q) / det;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	hitRecord.t0 = t;
	Vector3 faceNormal = Utilities::normalize(n);
	if (this->_hasVertexNormals)
	{
		const Vector3 interpolatedNormal =
			(this->_normal0 * (1.0 - u - v))
			+ (this->_normal1 * u)
			+ (this->_normal2 * v);
		hitRecord.normal = normalizedOrFallback(interpolatedNormal, faceNormal);
	}
	else
	{
		hitRecord.normal = faceNormal;
	}
	hitRecord.normal = orientAgainstRay(hitRecord.normal, ray);
	hitRecord.material = this->_material;
	hitRecord.position = ray.pointAtRay(hitRecord.t0);

	return (true);
}

// Creates an AABB / bounding box for this Triangle
bool	Triangle::createBoundingBox(AABB& outputBoundingBox) const
{
	Vector3 minimum = this->_vertex0;
	Vector3 maximum = this->_vertex0;

	std::vector<Vector3> vectors;
	vectors.push_back(this->_vertex0);
	vectors.push_back(this->_vertex1);
	vectors.push_back(this->_vertex2);

	for (Vector3 vector : vectors)
	{
		if (vector.getX() < minimum.getX())
		{
			minimum.setX(vector.getX());
		}
		if (vector.getY() < minimum.getY())
		{
			minimum.setY(vector.getY());
		}
		if (vector.getZ() < minimum.getZ())
		{
			minimum.setZ(vector.getZ());
		}

		if (vector.getX() > maximum.getX())
		{
			maximum.setX(vector.getX());
		}
		if (vector.getY() > maximum.getY())
		{
			maximum.setY(vector.getY());
		}
		if (vector.getZ() > maximum.getZ())
		{
			maximum.setZ(vector.getZ());
		}
	}

	minimum.setX(minimum.getX() - T_MIN);
	minimum.setY(minimum.getY() - T_MIN);
	minimum.setZ(minimum.getZ() - T_MIN);
	maximum.setX(maximum.getX() + T_MIN);
	maximum.setY(maximum.getY() + T_MIN);
	maximum.setZ(maximum.getZ() + T_MIN);

	outputBoundingBox = AABB(minimum, maximum);

	return (true);
}

double	Triangle::area(void) const
{
	return (Utilities::vectorLength(Utilities::cross(this->_vertex1 - this->_vertex0, this->_vertex2 - this->_vertex0)) / 2.0);
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

	Vector3 geometricNormal = Utilities::normalize(Utilities::cross(this->_vertex1 - this->_vertex0, this->_vertex2 - this->_vertex0));
	geometricNormal = orientAgainstRay(geometricNormal, ray);

	const double distanceSquared = hitRecord.t0 * hitRecord.t0 * Utilities::vectorLengthSquared(vec);
	const double cosine = std::fabs(Utilities::dot(vec, geometricNormal) / Utilities::vectorLength(vec));
	if (cosine <= 0.0)
	{
		return (0.0);
	}

	return (distanceSquared / (cosine * triangleArea));
}

Vector3	Triangle::random(const Vector3& origin) const
{
	double u = randomEngine.doubleFloat();
	double v = randomEngine.doubleFloat();

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
