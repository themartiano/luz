#ifndef HITTABLE_HPP
#define HITTABLE_HPP

#include "AABB.hpp"
#include "Material.hpp"
#include "Ray.hpp"
#include "Vector3.hpp"

class   Hittable
{
    public:
        virtual ~Hittable(void) = default;
        virtual bool        hit(Ray& ray, double t_max) const = 0;
        virtual bool        createBoundingBox(AABB& outputBoundingBox) const = 0;
        virtual Material    getMaterial(void) const = 0;
        virtual double pdfValue(const Vector3& origin, const Vector3& vec) const;
        virtual Vector3 random(const Vector3& origin) const;
};

#endif