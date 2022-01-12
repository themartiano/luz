#pragma once

#include "Vector3.hpp"

class   ONB
{
	public:
		ONB(void) = default;
		ONB(const Vector3& n);
		Vector3 getU(void) const;
		Vector3 getV(void) const;
		Vector3 getW(void) const;
		Vector3 local(double a, double b, double c) const;
		Vector3 local(const Vector3& vec) const;

	private:
		Vector3 _axis[3];
};
