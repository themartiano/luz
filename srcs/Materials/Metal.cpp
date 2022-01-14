#include "Materials/Metal.hpp"
#include "Utilities.hpp"
#include "Vector3.hpp"

bool	Metal::scatter(Ray& ray)
{
	Vector3	reflected = Utilities::reflect(Utilities::normalize(ray.getDirection()), ray.hitRecord.normal);

	ray.scatterRecord.specularRay = std::make_unique<Ray>(ray.hitRecord.position, reflected + this->_reflectionFuzziness * Utilities::randomPointInsideUnitSphere());
	ray.scatterRecord.attenuation = this->_color;
	ray.scatterRecord.isSpecular = true;
	ray.scatterRecord.pdfPtr = nullptr;
}
