#ifndef AABB_HPP
#define AABB_HPP

#include "Vector3.hpp"
#include "Ray.hpp"

class   AABB
{
    public:
        AABB(void);
        AABB(Vector3 minimum, Vector3 maximum);
        Vector3 getMinimum(void) const;
        Vector3 getMaximum(void) const;
        bool    hit(Ray& ray, double t_max) const;

    private:
        Vector3 _minimum;
        Vector3 _maximum;
};

#endif