#ifndef SQUARE_HPP
#define SQUARE_HPP

#include "Transform.hpp"
#include "Material.hpp"

class	Square
{
	public:
		Square(void);
		Square(Transform transform, Material material, float hss);

	private:
		Transform	_transform;
		Material	_material;
		float		_halfSideSize;
};

#endif