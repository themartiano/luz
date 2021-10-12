#ifndef SPHERE_HPP
# define SPHERE_HPP

#include "Transform.hpp"
#include "Material.hpp"

class	Sphere
{
	public:
		Sphere(void);
		Sphere(Transform transform, Material material, float radius);

	private:
		Transform	_transform;
		Material	_material;
		float		_radius;
};

#endif