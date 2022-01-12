#pragma once

#include "Hittable.hpp"
#include "Vector3.hpp"
#include "Ray.hpp"
#include "AABB.hpp"
#include "Material.hpp"

class   Triangle : public Hittable
{
	public:
		Triangle(void);
		Triangle(Vector3 vertex0, Vector3 vertex1, Vector3 vertex2, Material material);
		void			setVertex0(Vector3 vertex0);
		void			setVertex1(Vector3 vertex1);
		void			setVertex2(Vector3 vertex2);
		virtual Material getMaterial(void) const override;
		void			setMaterial(Material material);
		virtual bool	hit(Ray& ray, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		Vector3	 _vertex0;
		Vector3	 _vertex1;
		Vector3	 _vertex2;
		Material	_material;
};
