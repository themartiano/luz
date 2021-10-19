#ifndef UTILITIES_HPP
# define UTILITIES_HPP

#include "Vector3.hpp"
#include "Objects/Sphere.hpp"
#include <cmath>

float	dot(Vector3 vec1, Vector3 vec2);
Vector3	cross(Vector3 vec1, Vector3 vec2);
float	vectorLengthSQRT(Vector3 vector);
float	vectorLength(Vector3 vector);
Vector3	normalize(Vector3 vector);
Vector3 randomPointInsideUnitSphere(void);

#endif