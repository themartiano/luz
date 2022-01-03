#include "PDFs/HittablePDF.hpp"
#include "Utilities.hpp"

HittablePDF::HittablePDF(std::shared_ptr<Hittable> hittable, const Vector3& origin)
{
    this->_hittable = hittable;
    this->_origin = origin;
}

HittablePDF::HittablePDF(std::vector<std::shared_ptr<Hittable>> hittables, const Vector3& origin)
{
    int randomIndex = randomInt(0, hittables.size() - 1);

    this->_hittable = hittables.at(randomIndex);
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
