#pragma once

#include "Color.hpp"
#include "Ray.hpp"
#include <memory>

class	Ray;

struct ScatterRecord
{
	std::unique_ptr<Ray>	specularRay;
	bool					isSpecular;
	Color					attenuation;
	std::shared_ptr<PDF>	pdfPtr;
};
