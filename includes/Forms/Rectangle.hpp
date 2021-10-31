#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include "Hittable.hpp"
#include "Vector3.hpp"
#include "Material.hpp"
#include "AABB.hpp"
#include "Transform.hpp"

class   Rectangle : public Hittable
{
    public:
        Rectangle(void);
        Rectangle(Transform transform, Material material, double width, double height);
		virtual bool    hit(Ray& ray, double t_max) const override;
		virtual bool    createBoundingBox(AABB& outputBoundingBox) const override;

    private:
        Transform   _transform;
        Material    _material;
        double       _width;
        double       _height;
};

#endif