#ifndef SPHERE_HPP
# define SPHERE_HPP

#include "Transform.hpp"
#include "Material.hpp"

class	Sphere
{
	public:
		Sphere(void);
		Sphere(Vector3 position, Material material, float radius);
		Vector3	getPosition(void) const;
		Material		getMaterial(void) const;
		float		getRadius(void) const;

	private:
		Vector3		_position;
		Material	_material;
		float		_radius;
};

#endif