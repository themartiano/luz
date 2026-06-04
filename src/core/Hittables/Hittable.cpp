#include "Hittables/Hittable.hpp"
#include "Defaults.hpp"

double Hittable::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	return (0.0);
	(void)origin;
	(void)vec;
}

Vector3 Hittable::random(const Vector3& origin) const
{
	return (Vector3(1.0, 0.0, 0.0));
	(void)origin;
}
