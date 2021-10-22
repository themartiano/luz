#include "Forms/Triangle.hpp"

/*
	Constructors
*/

// Constructs the Triangle with default values
Triangle::Triangle(void)
{
	this->_position1 = Vector3(0.0f, 0.0f, 0.0f); //Values
	this->_position2 = Vector3(0.0f, 0.0f, 0.0f); //Values
	this->_position3 = Vector3(0.0f, 0.0f, 0.0f); //Values
	this->_material = Material(Color(0.49f, 0.49f, 0.49f, 0.0f), 1.0f, 0.0f, 0.5f);
}

// Constructs the Triangle with custom values
Triangle::Triangle(Vector3 pos1, Vector3 pos2, Vector3 pos3, Material material)
{
	this->_position1 = pos1;
	this->_position2 = pos2;
	this->_position3 = pos3;
	this->_material = material;
}
