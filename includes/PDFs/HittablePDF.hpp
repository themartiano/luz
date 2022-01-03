#ifndef HITTABLEPDF_HPP
#define HITTABLEPDF_HPP

#include "PDFs/PDF.hpp"
#include "Vector3.hpp"
#include "Hittable.hpp"
#include <memory>
#include <vector>

class   HittablePDF : public PDF
{
    public:
        HittablePDF(std::shared_ptr<Hittable> hittable, const Vector3& origin);
        HittablePDF(std::vector<std::shared_ptr<Hittable>> hittables, const Vector3& origin);
        virtual double value(const Vector3& direction) const override;
        virtual Vector3 generate(void) const override;

    private:
        Vector3 _origin;
        std::shared_ptr<Hittable> _hittable;
};

#endif