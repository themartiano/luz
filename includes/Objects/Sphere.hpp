#ifndef SPHERE_HPP
# define SPHERE_HPP

#include "Transform.hpp"
#include "Material.hpp"
#include "Utilities.hpp"

class	Sphere
{
	public:
		Sphere(void);
		Sphere(Transform transform, Material material, float radius);
		Material	getMaterial(void) const;
		Transform	getTransform(void) const;
		float		getRadius(void) const;

	private:
		Transform	_transform;
		Material	_material;
		float		_radius;
};

#endif