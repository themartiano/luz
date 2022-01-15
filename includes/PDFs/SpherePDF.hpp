#pragma once

#include "PDFs/PDF.hpp"
#include "Vector3.hpp"
#include "ONB.hpp"

class   SpherePDF : public PDF
{
	public:
		SpherePDF(void) = default;
		virtual double value (const Vector3& direction) const override;
		virtual Vector3 generate(void) const override;
};
