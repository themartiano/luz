#include "ONB.hpp"
#include "Utilities.hpp"
#include <cmath>

ONB::ONB(const Vector3& n)
{
	this->_axis[2] = Utilities::normalize(n);

	Vector3 a = (fabs(getW().getX()) > 0.9) ? Vector3(0.0, 1.0, 0.0) : Vector3(1.0, 0.0, 0.0);

	this->_axis[1] = Utilities::normalize(Utilities::cross(getW(), a));
	this->_axis[0] = Utilities::cross(getW(), getV());
}

Vector3 ONB::getU(void) const
{
	return (this->_axis[0]);
}

Vector3 ONB::getV(void) const
{
	return (this->_axis[1]);
}

Vector3 ONB::getW(void) const
{
	return (this->_axis[2]);
}

Vector3 ONB::local(double a, double b, double c) const
{
	return (a * getU() + b * getV() + c * getW());
}

Vector3 ONB::local(const Vector3& vec) const
{
	return (vec.getX() * getU() + vec.getY() * getV() + vec.getZ() * getW());
}

