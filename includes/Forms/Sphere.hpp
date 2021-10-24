#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "Transform.hpp"
#include "Material.hpp"
#include "AABB.hpp"
#include "Hittable.hpp"

class	Sphere : public Hittable
{
	public:
		Sphere(void);
		Sphere(Vector3 position, Material material, float radius);
		Vector3				getPosition(void) const;
		float				getRadius(void) const;
		virtual bool    	hit(Ray& ray, float t_max) const override;
		virtual bool    	createBoundingBox(AABB& outputBoundingBox) const override;
		virtual Material	getMaterial(void) const override;

	private:
		Vector3		_position;
		Material	_material;
		float		_radius;
};

#endif