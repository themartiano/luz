#ifndef MIXTUREPDF_HPP
#define MIXTUREPDF_HPP

#include "PDFs/PDF.hpp"
#include "Vector3.hpp"
#include <memory>

class   MixturePDF : public PDF
{
    public:
        MixturePDF(std::shared_ptr<PDF> pdf0, std::shared_ptr<PDF> pdf1);
        virtual double value(const Vector3& direction) const override;
        virtual Vector3 generate(void) const override;

    private:
        std::shared_ptr<PDF> _pdfs[2];
};

#endif