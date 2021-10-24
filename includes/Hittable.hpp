#ifndef HITTABLE_HPP
#define HITTABLE_HPP

#include "AABB.hpp"
#include "Material.hpp"

class   Hittable
{
    public:
        virtual ~Hittable(void) = default;
        virtual bool        hit(Ray& ray, float t_max) const = 0;
        virtual bool        createBoundingBox(AABB& outputBoundingBox) const = 0;
        virtual Material    getMaterial(void) const = 0;
};

#endif