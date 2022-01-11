#include "PDFs/CosinePDF.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <cmath>

CosinePDF::CosinePDF(const Vector3& w)
{
	this->_uvw = ONB(w);
}

double CosinePDF::value(const Vector3& direction) const
{
	double cosine = Utilities::dot(Utilities::normalize(direction), this->_uvw.getW());

	return ((cosine <= 0.0) ? 0.0 : cosine / D_PI);
}

Vector3 CosinePDF::generate(void) const
{
	return (this->_uvw.local(Utilities::randomCosineDirection()));
}
