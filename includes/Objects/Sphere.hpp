#ifndef SPHERE_HPP
# define SPHERE_HPP

#include "Transform.hpp"
#include "Material.hpp"
#include "Ray.hpp"
#include "Utilities.hpp"

class	Sphere
{
	public:
		Sphere(void);
		Sphere(Transform transform, Material material, float radius);
		bool		hit(Ray ray);
		Material	getMaterial(void) const;

	private:
		Transform	_transform;
		Material	_material;
		float		_radius;
};

#endif