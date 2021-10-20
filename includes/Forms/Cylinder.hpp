#ifndef CYLINDER_HPP
# define CYLINDER_HPP

#include "Transform.hpp"
#include "Material.hpp"

class	Cylinder
{
	public:
		Cylinder(void);
		Cylinder(Transform transform, Material material, float radius, float height);

	private:
		Transform	_transform;
		Material	_material;
		float		_radius;
		float		_height;
};

#endif