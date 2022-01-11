#ifndef COSINEPDF_HPP
#define COSINEPDF_HPP

#include "PDFs/PDF.hpp"
#include "Vector3.hpp"
#include "ONB.hpp"

class   CosinePDF : public PDF
{
	public:
		CosinePDF(const Vector3& w);
		virtual double value (const Vector3& direction) const override;
		virtual Vector3 generate(void) const override;

	private:
		ONB _uvw;
};

#endif