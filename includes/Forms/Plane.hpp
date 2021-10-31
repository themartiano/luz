#ifndef PLANE_HPP
#define PLANE_HPP

#include "Hittable.hpp"
#include "Vector3.hpp"
#include "Material.hpp"

class   Plane : public Hittable
{
    public:
        Plane(void);
        Plane(double y, Vector3 orientation, Material material);
        virtual bool    hit(Ray& ray, double t_max) const override;
        virtual bool    createBoundingBox(AABB& boundingBox) const override;

    private:
        double       _y;
        Vector3     _orientation;
        Material    _material;

};

#endif