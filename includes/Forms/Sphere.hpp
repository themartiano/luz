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
		Sphere(Vector3 position, Material material, double radius);
		Vector3				getPosition(void) const;
		double				getRadius(void) const;
		virtual bool    	hit(Ray& ray, double t_max) const override;
		virtual bool    	createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		Vector3		_position;
		Material	_material;
		double		_radius;
};

#endif