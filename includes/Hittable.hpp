#ifndef HITTABLE_HPP
#define HITTABLE_HPP

#include "AABB.hpp"
#include "Material.hpp"

class   Hittable
{
    public:
        virtual bool        hit(Ray& ray, float t_max) const = 0;
        virtual AABB        createBoundingBox(void) const = 0;
        virtual Material    getMaterial(void) const = 0;
};

#endif