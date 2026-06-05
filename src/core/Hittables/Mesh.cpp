#include "Hittables/Mesh.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include "Random.hpp"
#include "Defaults.hpp"
#include <cmath>
#include <memory>

/*
	Constructors
*/

// Constructs the Mesh with default values
Mesh::Mesh(void)
{
	this->_position = Vector3();
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_bvh = BVHNode();
	this->_triangles = std::vector<std::shared_ptr<Hittable>>();
	this->_triangleAreaPrefixSums = std::vector<double>();
	this->_totalArea = 0.0;
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, BVHNode bvh)
{
	this->_position = position;
	this->_material = material;
	this->_bvh = bvh;
	this->_triangles = std::vector<std::shared_ptr<Hittable>>();
	this->_triangleAreaPrefixSums = std::vector<double>();
	this->_totalArea = 0.0;
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<std::shared_ptr<Hittable>> triangles)
{
	this->_position = position;
	this->_material = material;
	this->_triangles = triangles;
	this->_bvh = BVHNode(triangles);
	this->_computeTriangleAreas();
}

void	Mesh::_computeTriangleAreas(void)
{
	this->_triangleAreaPrefixSums.clear();
	this->_triangleAreaPrefixSums.reserve(this->_triangles.size());
	this->_totalArea = 0.0;

	for (const std::shared_ptr<Hittable>& hittable : this->_triangles)
	{
		const std::shared_ptr<Triangle> triangle = std::dynamic_pointer_cast<Triangle>(hittable);

		if (triangle)
		{
			this->_totalArea += triangle->area();
		}
		this->_triangleAreaPrefixSums.push_back(this->_totalArea);
	}
}

// Returns the Mesh's material
std::shared_ptr<Material>	Mesh::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the Mesh's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Mesh::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	return (this->_bvh.hit(ray, hitRecord, t_min, t_max));
}

// Returns the AABB / bounding box for this Mesh's BVH
bool	Mesh::createBoundingBox(AABB& outputBoundingBox) const
{
	this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
}

double	Mesh::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	if (this->_triangles.empty() || this->_totalArea <= 0.0)
	{
		return (0.0);
	}

	Ray ray(origin, vec);
	HitRecord hitRecord;

	if (!this->hit(ray, hitRecord, T_MIN, T_MAX))
	{
		return (0.0);
	}

	const double distanceSquared = hitRecord.t0 * hitRecord.t0 * Utilities::vectorLengthSquared(vec);
	const double cosine = std::fabs(Utilities::dot(vec, hitRecord.normal) / Utilities::vectorLength(vec));
	if (cosine <= 0.0)
	{
		return (0.0);
	}

	return (distanceSquared / (cosine * this->_totalArea));
}

Vector3	Mesh::random(const Vector3& origin) const
{
	if (this->_triangles.empty() || this->_totalArea <= 0.0)
	{
		return (Hittable::random(origin));
	}

	const double targetArea = randomEngine.doubleFloat() * this->_totalArea;
	std::size_t randomIndex = 0;

	while (
		randomIndex + 1 < this->_triangleAreaPrefixSums.size()
		&& this->_triangleAreaPrefixSums.at(randomIndex) < targetArea
	)
	{
		randomIndex++;
	}

	return (this->_triangles.at(randomIndex)->random(origin));
}
