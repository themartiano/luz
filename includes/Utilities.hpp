#ifndef UTILITIES_HPP
# define UTILITIES_HPP

#include "Vector3.hpp"
#include <string>

float	    dot(Vector3 vector1, Vector3 vector2);
Vector3	    cross(Vector3 vector1, Vector3 vector2);
float	    vectorLengthNoSQRT(Vector3 vector);
float	    vectorLength(Vector3 vector);
Vector3	    normalize(Vector3 vector);
Vector3     randomPointInsideUnitSphere(void);
std::string pluralOrSingular(int number);

#endif