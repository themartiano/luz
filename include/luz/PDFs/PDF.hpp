#pragma once

#include "Vector3.hpp"

class   PDF
{
	public:
		virtual ~PDF(void) = default;

		virtual double value(const Vector3& direction) const = 0;
		virtual Vector3 generate(void) const = 0;
};
