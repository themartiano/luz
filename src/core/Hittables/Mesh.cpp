#include "Hittables/Mesh.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include "Defaults.hpp"
#include "Sampler.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <utility>

namespace
{
	constexpr std::size_t	MESH_BVH_LEAF_SIZE = 8;
	constexpr std::size_t	MESH_BVH_STACK_SIZE = 128;

	double	boxMinimumAxis(const AABB& box, int axis)
	{
		return (box.getMinimum()[axis]);
	}

	Vector3	boxCentroid(const AABB& box)
	{
		return ((box.getMinimum() + box.getMaximum()) / 2.0);
	}

	AABB	mergeBoxes(const AABB& first, const AABB& second)
	{
		return (AABB(
			Vector3(
				std::min(first.getMinimum().getX(), second.getMinimum().getX()),
				std::min(first.getMinimum().getY(), second.getMinimum().getY()),
				std::min(first.getMinimum().getZ(), second.getMinimum().getZ())
			),
			Vector3(
				std::max(first.getMaximum().getX(), second.getMaximum().getX()),
				std::max(first.getMaximum().getY(), second.getMaximum().getY()),
				std::max(first.getMaximum().getZ(), second.getMaximum().getZ())
			)
		));
	}

	AABB	boundingBoxForRange(
		const std::vector<AABB>& boxes,
		const std::vector<std::size_t>& indices,
		std::size_t start,
		std::size_t end
	)
	{
		AABB result;
		bool hasBox = false;

		for (std::size_t i = start; i < end; i++)
		{
			const AABB& box = boxes[indices[i]];

			result = hasBox ? mergeBoxes(result, box) : box;
			hasBox = true;
		}

		return (result);
	}

	int	largestExtentAxis(const AABB& box)
	{
		const Vector3 extent = box.getMaximum() - box.getMinimum();

		if (extent.getX() >= extent.getY() && extent.getX() >= extent.getZ())
		{
			return (0);
		}
		if (extent.getY() >= extent.getZ())
		{
			return (1);
		}
		return (2);
	}
}

/*
	Constructors
*/

// Constructs the Mesh with default values
Mesh::Mesh(void)
{
	this->_position = Vector3();
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_legacyBVH = BVHNode();
	this->_totalArea = 0.0;
	this->_usesPackedTriangles = false;
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, BVHNode bvh)
{
	this->_position = position;
	this->_material = material;
	this->_legacyBVH = bvh;
	this->_totalArea = 0.0;
	this->_usesPackedTriangles = false;
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<std::shared_ptr<Hittable>> triangles)
{
	this->_position = position;
	this->_material = material;
	this->_totalArea = 0.0;
	this->_usesPackedTriangles = false;

	if (this->_trySetPackedTriangles(triangles))
	{
		return;
	}

	this->_legacyTriangles = std::move(triangles);
	this->_legacyBVH = BVHNode(this->_legacyTriangles);
	this->_computeLegacyTriangleAreas();
}

Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<Triangle> triangles)
{
	this->_position = position;
	this->_material = material;
	this->_totalArea = 0.0;
	this->_usesPackedTriangles = false;
	this->_setPackedTriangles(std::move(triangles));
}

void	Mesh::_setPackedTriangles(std::vector<Triangle> triangles)
{
	this->_legacyTriangles.clear();
	this->_triangleBoundingBoxes.clear();
	this->_triangleCentroids.clear();
	this->_triangleIndices.clear();
	this->_packedBVHNodes.clear();
	this->_triangleAreaPrefixSums.clear();
	this->_triangles.clear();
	this->_triangles.reserve(triangles.size());

	for (Triangle& triangle : triangles)
	{
		if (triangle.area() > 0.0)
		{
			this->_triangles.push_back(std::move(triangle));
		}
	}

	this->_usesPackedTriangles = true;
	this->_computePackedTriangleAreas();
	this->_buildPackedBVH();
}

bool	Mesh::_trySetPackedTriangles(const std::vector<std::shared_ptr<Hittable>>& triangles)
{
	std::vector<Triangle> packedTriangles;

	packedTriangles.reserve(triangles.size());
	for (const std::shared_ptr<Hittable>& hittable : triangles)
	{
		const std::shared_ptr<Triangle> triangle = std::dynamic_pointer_cast<Triangle>(hittable);

		if (!triangle)
		{
			return (false);
		}
		packedTriangles.push_back(*triangle);
	}

	this->_setPackedTriangles(std::move(packedTriangles));
	return (true);
}

void	Mesh::_buildPackedBVH(void)
{
	this->_triangleBoundingBoxes.clear();
	this->_triangleCentroids.clear();
	this->_triangleIndices.clear();
	this->_packedBVHNodes.clear();

	if (this->_triangles.empty())
	{
		return;
	}

	this->_triangleBoundingBoxes.reserve(this->_triangles.size());
	this->_triangleCentroids.reserve(this->_triangles.size());
	this->_triangleIndices.resize(this->_triangles.size());
	std::iota(this->_triangleIndices.begin(), this->_triangleIndices.end(), 0);

	for (const Triangle& triangle : this->_triangles)
	{
		AABB boundingBox;

		triangle.createBoundingBox(boundingBox);
		this->_triangleBoundingBoxes.push_back(boundingBox);
		this->_triangleCentroids.push_back(boxCentroid(boundingBox));
	}

	this->_packedBVHNodes.reserve(this->_triangles.size() * 2);
	this->_buildPackedBVHNode(0, this->_triangleIndices.size());
}

std::size_t	Mesh::_buildPackedBVHNode(std::size_t start, std::size_t end)
{
	const std::size_t nodeIndex = this->_packedBVHNodes.size();
	const std::size_t triangleCount = end - start;
	const AABB nodeBox = boundingBoxForRange(
		this->_triangleBoundingBoxes,
		this->_triangleIndices,
		start,
		end
	);

	this->_packedBVHNodes.push_back(PackedBVHNode());
	this->_packedBVHNodes[nodeIndex].boundingBox = nodeBox;
	if (nodeIndex == 0)
	{
		this->_boundingBox = nodeBox;
	}

	if (triangleCount <= MESH_BVH_LEAF_SIZE)
	{
		this->_packedBVHNodes[nodeIndex].start = start;
		this->_packedBVHNodes[nodeIndex].count = triangleCount;
		this->_packedBVHNodes[nodeIndex].isLeaf = true;
		return (nodeIndex);
	}

	const int axis = largestExtentAxis(nodeBox);
	const std::size_t mid = start + triangleCount / 2;

	std::nth_element(
		this->_triangleIndices.begin() + start,
		this->_triangleIndices.begin() + mid,
		this->_triangleIndices.begin() + end,
		[this, axis](std::size_t first, std::size_t second)
		{
			const double firstCentroid = this->_triangleCentroids[first][axis];
			const double secondCentroid = this->_triangleCentroids[second][axis];

			if (firstCentroid == secondCentroid)
			{
				return (boxMinimumAxis(this->_triangleBoundingBoxes[first], axis)
					< boxMinimumAxis(this->_triangleBoundingBoxes[second], axis));
			}
			return (firstCentroid < secondCentroid);
		}
	);

	this->_packedBVHNodes[nodeIndex].left = this->_buildPackedBVHNode(start, mid);
	this->_packedBVHNodes[nodeIndex].right = this->_buildPackedBVHNode(mid, end);

	return (nodeIndex);
}

void	Mesh::_computePackedTriangleAreas(void)
{
	this->_triangleAreaPrefixSums.clear();
	this->_triangleAreaPrefixSums.reserve(this->_triangles.size());
	this->_totalArea = 0.0;

	for (const Triangle& triangle : this->_triangles)
	{
		this->_totalArea += triangle.area();
		this->_triangleAreaPrefixSums.push_back(this->_totalArea);
	}
}

void	Mesh::_computeLegacyTriangleAreas(void)
{
	this->_triangleAreaPrefixSums.clear();
	this->_triangleAreaPrefixSums.reserve(this->_legacyTriangles.size());
	this->_totalArea = 0.0;

	for (const std::shared_ptr<Hittable>& hittable : this->_legacyTriangles)
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
	if (!this->_usesPackedTriangles)
	{
		return (this->_legacyBVH.hit(ray, hitRecord, t_min, t_max));
	}
	if (this->_packedBVHNodes.empty())
	{
		return (false);
	}

	std::array<std::size_t, MESH_BVH_STACK_SIZE> stack;
	std::size_t stackSize = 0;
	bool hitAnything = false;
	double closestHit = t_max;

	stack[stackSize++] = 0;
	while (stackSize > 0)
	{
		const std::size_t nodeIndex = stack[--stackSize];
		const PackedBVHNode& node = this->_packedBVHNodes[nodeIndex];
		HitRecord boundingHitRecord;

		if (!node.boundingBox.hit(ray, boundingHitRecord, closestHit))
		{
			continue;
		}

		if (node.isLeaf)
		{
			for (std::size_t i = node.start; i < node.start + node.count; i++)
			{
				const Triangle& triangle = this->_triangles[this->_triangleIndices[i]];
				HitRecord triangleHitRecord;

				if (triangle.hit(ray, triangleHitRecord, t_min, closestHit))
				{
					hitRecord = triangleHitRecord;
					closestHit = triangleHitRecord.t0;
					hitAnything = true;
				}
			}
			continue;
		}

		stack[stackSize++] = node.left;
		stack[stackSize++] = node.right;
	}

	return (hitAnything);
}

// Returns the AABB / bounding box for this Mesh's BVH
bool	Mesh::createBoundingBox(AABB& outputBoundingBox) const
{
	if (!this->_usesPackedTriangles)
	{
		return (this->_legacyBVH.createBoundingBox(outputBoundingBox));
	}
	if (this->_packedBVHNodes.empty())
	{
		return (false);
	}

	outputBoundingBox = this->_boundingBox;
	return (true);
}

double	Mesh::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	if (this->_triangleAreaPrefixSums.empty() || this->_totalArea <= 0.0)
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
	if (this->_triangleAreaPrefixSums.empty() || this->_totalArea <= 0.0)
	{
		return (Hittable::random(origin));
	}

	const double targetArea = Sampler::sample1D(Sampler::DIM_LIGHT_SURFACE_SELECTION) * this->_totalArea;
	const auto areaIt = std::lower_bound(
		this->_triangleAreaPrefixSums.begin(),
		this->_triangleAreaPrefixSums.end(),
		targetArea
	);
	const std::size_t randomIndex = std::min<std::size_t>(
		static_cast<std::size_t>(areaIt - this->_triangleAreaPrefixSums.begin()),
		this->_triangleAreaPrefixSums.size() - 1
	);

	if (this->_usesPackedTriangles)
	{
		return (this->_triangles[randomIndex].random(origin));
	}

	return (this->_legacyTriangles[randomIndex]->random(origin));
}
