#include "Materials/Metal.hpp"
#include "Utilities.hpp"
#include "Vector3.hpp"
#include "Random.hpp"

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

bool	Metal::scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord)
{
	Vector3	reflected = Utilities::reflect(ray.getDirection(), hitRecord.normal);

	scatterRecord.specularRay = Ray(hitRecord.position, reflected + this->_reflectionFuzziness * randomEngine.pointInsideUnitSphere());
	scatterRecord.attenuation = this->_color;
	scatterRecord.isSpecular = true;
	scatterRecord.pdfType = SCATTER_PDF_NONE;

	return (true);
}

MaterialType	Metal::getType(void) const
{
	return (METAL);
}
