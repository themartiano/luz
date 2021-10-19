#ifndef HITUTILS_HPP
# define HITUTILS_HPP

#include "Ray.hpp"
#include "Defaults.hpp"

bool	hitSphere(Ray& ray, Sphere sphere, float t_max);

#endif