#include "PDFs/HittablePDF.hpp"
#include "Utilities.hpp"
#include "Random.hpp"
#include <stdexcept>

HittablePDF::HittablePDF(std::shared_ptr<Hittable> hittable, const Vector3& origin)
{
	if (!hittable)
	{
		throw std::invalid_argument("HittablePDF requires a hittable.");
	}

	this->_hittables.push_back(hittable);
	this->_origin = origin;
}

HittablePDF::HittablePDF(std::vector<std::shared_ptr<Hittable>> hittables, const Vector3& origin)
{
	if (hittables.empty())
	{
		throw std::invalid_argument("HittablePDF requires at least one hittable.");
	}

	this->_hittables = hittables;
	this->_origin = origin;
}

double HittablePDF::value(const Vector3& direction) const
{
	double sum = 0.0;

	for (const std::shared_ptr<Hittable>& hittable : this->_hittables)
	{
		sum += hittable->pdfValue(this->_origin, direction);
	}

	return (sum / this->_hittables.size());
}

Vector3 HittablePDF::generate(void) const
{
	unsigned int randomIndex = randomEngine.integer(0, this->_hittables.size() - 1);

	return (this->_hittables.at(randomIndex)->random(this->_origin));
}
