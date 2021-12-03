#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include "Vector3.hpp"
#include "Scene.hpp"
#include <string>

double	    dot(Vector3 vector1, Vector3 vector2);
Vector3	    cross(Vector3 vector1, Vector3 vector2);
double	    vectorLengthNoSQRT(Vector3 vector);
double	    vectorLength(Vector3 vector);
Vector3	    normalize(Vector3 vector);
Vector3     randomPointInsideUnitSphere(void);
Vector3     randomPointInsideUnitDisk(void);
Vector3 	reflect(Vector3 vector, Vector3 normal);
bool        refract(Vector3 vector, Vector3 normal, double refractiveIndex, Vector3& refractedVector);
double	    schlick(double cosine, double refractiveIndex);
std::string pluralOrSingular(int number);
void	    setdoubleRange(double& flt, double min, double max);
bool	    createMainBoundingBox(Scene& scene, AABB& newBoundingBox);
AABB	    mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2);

inline double	randomDouble(void)
{
    return (rand() / (RAND_MAX + 1.0));
}

inline double	randomDouble(double min, double max)
{
    return (min + (max - min) * randomDouble());
}

inline int	randomInt(int min, int max)
{
    return (static_cast<int>(randomDouble(min, max + 1)));
}

#endif