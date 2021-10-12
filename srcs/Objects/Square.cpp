#include "Objects/Square.hpp"

/*
	Constructors
*/

// Constructs the Square with default values
Square::Square(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(126, 126, 126, 0), 1.0f);
	this->_half_side_size = 0.0f;
}

// Constructs the Square with custom values
Square::Square(Transform transform, Material material, float hss)
{
	this->_transform = transform;
	this->_material = material;
	this->_half_side_size = hss;
}
