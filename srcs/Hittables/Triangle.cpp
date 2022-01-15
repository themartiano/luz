#include "Hittables/Triangle.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Materials/Lambertian.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Triangle with default values
Triangle::Triangle(void)
{
	this->_vertex0 = Vector3(0.0, 1.0, 0.0);
	this->_vertex1 = Vector3(-1.0, 0.0, 0.0);
	this->_vertex2 = Vector3(1.0, 0.0, 0.0);
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
}

// Constructs the Triangle with custom values
Triangle::Triangle(Vector3 vertex0, Vector3 vertex1, Vector3 vertex2, std::shared_ptr<Material> material)
{
	this->_vertex0 = vertex0;
	this->_vertex1 = vertex1;
	this->_vertex2 = vertex2;
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
	Vector3 p = Utilities::cross(ray.getDirection(), v2);
	double  det = Utilities::dot(v1, p);

	// If det is near 0, they're parallel. If it's negative, the triangle is backfacing the camera.
	if (fabs(det) < t_min)
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
	Vector3 n = Utilities::cross(v1, v2);
	if (Utilities::dot(n, ray.getDirection()) > 0.0)
	{
		n = n * -1.0;
	}
	hitRecord.normal = n;
	hitRecord.material = this->_material;
	hitRecord.position = ray.pointAtRay(hitRecord.t0);

	return (true);
}

// Creates an AABB / bounding box for this Triangle
bool	Triangle::createBoundingBox(AABB& outputBoundingBox) const
{
	Vector3 minimum = Vector3(0.0, 0.0, 0.0);
	Vector3 maximum = Vector3(0.0, 0.0, 0.0);

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

	minimum.setZ(minimum.getZ() - T_MIN);
	maximum.setZ(maximum.getZ() + T_MIN);

	outputBoundingBox = AABB(minimum, maximum);

	return (true);
}
