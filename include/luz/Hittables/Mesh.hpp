#pragma once

#include "Hittables/Hittable.hpp"
#include "Hittables/BVHNode.hpp"
#include "Hittables/Triangle.hpp"

class	Mesh : public Hittable
{
	public:
		Mesh(void);
		Mesh(Vector3 position, std::shared_ptr<Material> material, BVHNode bvh);
		Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<std::shared_ptr<Hittable>> triangles);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3	random(const Vector3& origin) const override;

	private:
		void	_computeTriangleAreas(void);

		Vector3 _position;
		std::shared_ptr<Material>	_material;
		BVHNode	_bvh;
		std::vector<std::shared_ptr<Hittable>>	_triangles;
		std::vector<double>	_triangleAreaPrefixSums;
		double	_totalArea;
};
