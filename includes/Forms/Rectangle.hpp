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
        Rectangle(Transform transform, float sideSize, Material material);
        Transform       getTransform(void) const;
        float           getSideSize(void) const;
        Material        getMaterial(void) const;
		virtual bool    hit(Ray& ray, float t_max) const override;
		virtual bool    createBoundingBox(AABB& outputBoundingBox) const override;

    private:
        Transform   _transform;
        float       _sideSize;
        Material    _material;
};

#endif