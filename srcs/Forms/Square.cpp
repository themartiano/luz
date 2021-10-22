#include "Forms/Square.hpp"

/*
	Constructors
*/

// Constructs the Square with default values
Square::Square(void)
{
	this->_transform = Transform();
	this->_material = Material(Color(0.49f, 0.49f, 0.49f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, 0.0f);
	this->_halfSideSize = 0.0f;
}

// Constructs the Square with custom values
Square::Square(Transform transform, Material material, float hss)
{
	this->_transform = transform;
	this->_material = material;
	this->_halfSideSize = hss;
}
