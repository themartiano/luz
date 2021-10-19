#ifndef HITUTILS_HPP
# define HITUTILS_HPP

#include "Ray.hpp"
#include "Objects/Sphere.hpp"

bool	hitSphere(Ray& ray, Sphere sphere, float t_max);

#endif