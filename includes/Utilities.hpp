#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include "Vector3.hpp"
#include "Scene.hpp"
#include <string>

float	    dot(Vector3 vector1, Vector3 vector2);
Vector3	    cross(Vector3 vector1, Vector3 vector2);
float	    vectorLengthNoSQRT(Vector3 vector);
float	    vectorLength(Vector3 vector);
Vector3	    normalize(Vector3 vector);
Vector3     randomPointInsideUnitSphere(void);
Vector3     randomPointInsideUnitDisk(void);
Vector3 	reflect(Vector3 vector, Vector3 normal);
bool        refract(Vector3 vector, Vector3 normal, float refractiveIndex, Vector3& refractedVector);
float	    schlick(float cosine, float refractiveIndex);
std::string pluralOrSingular(int number);
void	    setFloatRange(float& flt, float min, float max);
bool	    createMainBoundingBox(Scene scene, AABB& newBoundingBox);
AABB	    mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2);

inline float	randomFloat(void)
{
    return (rand() / (RAND_MAX + 1.0));
}

inline float	randomFloat(float min, float max)
{
    return (min + (max - min) * randomFloat());
}

inline int	randomInt(int min, int max)
{
    return (static_cast<int>(randomFloat(min, max + 1)));
}

#endif