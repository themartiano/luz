#pragma once

#include "Hittable.hpp"
#include "BVHNode.hpp"
#include "Forms/Triangle.hpp"

class	Mesh : public Hittable
{
	public:
		Mesh(void);
		Mesh(Vector3 position, std::shared_ptr<Material> material, BVHNode bvh);
		Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<std::shared_ptr<Hittable>> triangles);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		Vector3 _position;
		std::shared_ptr<Material>	_material;
		BVHNode	_bvh;
};
