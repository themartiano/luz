#include "PDFs/MixturePDF.hpp"
#include "Utilities.hpp"
#include "Random.hpp"

MixturePDF::MixturePDF(std::shared_ptr<PDF> pdf0, std::shared_ptr<PDF> pdf1)
{
	this->_pdfs[0] = pdf0;
	this->_pdfs[1] = pdf1;
}

double MixturePDF::value(const Vector3& direction) const
{
	return (0.5 * this->_pdfs[0]->value(direction) + 0.5 * this->_pdfs[1]->value(direction));
}

Vector3 MixturePDF::generate(void) const
{
	if (Random::doubleFloat() < 0.5)
	{
		return (this->_pdfs[0]->generate());
	}
	else
	{
		return (this->_pdfs[1]->generate());
	}
}
