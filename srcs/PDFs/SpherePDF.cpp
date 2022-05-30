#include "PDFs/SpherePDF.hpp"
#include "Defaults.hpp"
#include "Random.hpp"

double SpherePDF::value(const Vector3& direction) const
{
	return (1.0 / (4.0 * D_PI));
	(void)direction;
}

Vector3 SpherePDF::generate(void) const
{
	return (Random::pointInsideUnitSphere());
}
