#pragma once

#include "Hittables/Hittable.hpp"
#include "Hittables/BVHNode.hpp"
#include "Hittables/Triangle.hpp"
#include <cstddef>
#include <vector>

class	Mesh : public Hittable
{
	public:
		Mesh(void);
		Mesh(Vector3 position, std::shared_ptr<Material> material, BVHNode bvh);
		Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<std::shared_ptr<Hittable>> triangles);
		Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<Triangle> triangles);
		virtual Material*	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		hitAny(Ray& ray, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3	random(const Vector3& origin) const override;
		virtual bool		sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual double	lightSelectionWeight(void) const override;

	private:
		struct	PackedBVHNode
		{
			AABB		boundingBox;
			std::size_t	left = 0;
			std::size_t	right = 0;
			std::size_t	start = 0;
			std::size_t	count = 0;
			bool		isLeaf = false;
		};

		void	_setPackedTriangles(std::vector<Triangle> triangles);
		bool	_trySetPackedTriangles(const std::vector<std::shared_ptr<Hittable>>& triangles);
		void	_buildPackedBVH(void);
		std::size_t	_buildPackedBVHNode(std::size_t start, std::size_t end);
		void	_computePackedTriangleAreas(void);
		void	_computeLegacyTriangleAreas(void);

		Vector3 _position;
		std::shared_ptr<Material>	_material;
		BVHNode	_legacyBVH;
		std::vector<std::shared_ptr<Hittable>>	_legacyTriangles;
		std::vector<Triangle>	_triangles;
		std::vector<AABB>	_triangleBoundingBoxes;
		std::vector<Vector3>	_triangleCentroids;
		std::vector<std::size_t>	_triangleIndices;
		std::vector<PackedBVHNode>	_packedBVHNodes;
		std::vector<double>	_triangleAreaPrefixSums;
		AABB	_boundingBox;
		double	_totalArea;
		bool	_usesPackedTriangles;
};
