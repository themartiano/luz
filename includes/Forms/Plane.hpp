#ifndef PLANE_HPP
#define PLANE_HPP

#include "Transform.hpp"
#include "Material.hpp"

class	Plane
{
	public:
		Plane(void);
		Plane(Transform transform, Material material);

	private:
		Transform	_transform;
		Material	_material;
};

#endif