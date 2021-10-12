#include "Square.hpp"

/*
	Constructors
*/

Square::Square(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(126, 126, 126, 0), 1.0f);
	this->_half_side_size = 0.0f;
}

Square::Square(Transform transform, Material material, float hss)
{
	this->_transform = transform;
	this->_material = material;
	this->_half_side_size = hss;
}
