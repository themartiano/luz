#include "Materials/Metal.hpp"
#include "Utilities.hpp"
#include "Vector3.hpp"

Metal::Metal(void)
{
	this->_color = Color(0.6, 0.6, 0.6);
}

Metal::Metal(Color color)
{
	this->_color = color;
}

Metal::Metal(double reflectionFuzziness)
{
	this->_color = Color(0.6, 0.6, 0.6);
	this->_reflectionFuzziness = reflectionFuzziness;
}

Metal::Metal(Color color, double reflectionFuzziness)
{
	this->_color = color;
	this->_reflectionFuzziness = reflectionFuzziness;
}

bool	Metal::scatter(Ray& ray)
{
	Vector3	reflected = Utilities::reflect(Utilities::normalize(ray.getDirection()), ray.hitRecord.normal);

	ray.scatterRecord.specularRay = std::make_unique<Ray>(ray.hitRecord.position, reflected + this->_reflectionFuzziness * Utilities::randomPointInsideUnitSphere());
	ray.scatterRecord.attenuation = this->_color;
	ray.scatterRecord.isSpecular = true;
	ray.scatterRecord.pdfPtr = nullptr;

	return (true);
}

MaterialType	Metal::getType(void) const
{
	return (METAL);
}
