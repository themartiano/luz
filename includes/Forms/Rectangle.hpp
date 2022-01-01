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
        Rectangle(const Rectangle& toCopy);
        Rectangle(Transform transform, double width, double height, Material material);
        void            setTransform(Transform transform);
        Material        getMaterial(void) const;
        void            setMaterial(Material material);
        void            setWidth(double width);
        void            setHeight(double height);
		virtual bool    hit(Ray& ray, double t_max) const override;
		virtual bool    createBoundingBox(AABB& outputBoundingBox) const override;
        virtual double  pdfValue(const Vector3& origin, const Vector3& vec) const override;
        virtual Vector3 random(const Vector3& origin) const override;

    private:
        Transform   _transform;
        Material    _material;
        double      _width;
        double      _height;
};

#endif