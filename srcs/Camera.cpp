#include "Camera.hpp"

/*
	Constructors
*/

Camera::Camera(void)
{
	this->_transform = Transform();
	this->_fov = 70;
}

Camera::Camera(Transform transform, short fov)
{
	this->_transform = transform;
	this->_fov = fov;
}
