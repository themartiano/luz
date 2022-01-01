#include "HittablePDF.hpp"

HittablePDF::HittablePDF(std::shared_ptr<Hittable> hittable, const Vector3& origin)
{
    this->_hittable = hittable;
    this->_origin = origin;
}

double HittablePDF::value(const Vector3& direction) const
{
    return (this->_hittable->pdfValue(this->_origin, direction));
}

Vector3 HittablePDF::generate(void) const
{
    return (this->_hittable->random(this->_origin));
}
