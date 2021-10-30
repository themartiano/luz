#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include "Hittable.hpp"
#include "Vector3.hpp"
#include "Material.hpp"
#include "AABB.hpp"

class   Rectangle : public Hittable
{
    public:
        Rectangle(void);
        Rectangle(Vector3 position, float sideSize, Material material);
        Vector3         getPosition(void) const;
        float           getSideSize(void) const;
        Material        getMaterial(void) const;
		virtual bool    hit(Ray& ray, float t_max) const override;
		virtual bool    createBoundingBox(AABB& outputBoundingBox) const override;

    private:
        Vector3     _position;
        float       _sideSize;
        Material    _material;
        float       _x0;
        float       _x1;
        float       _y0;
        float       _y1;
        void    _calculateCoordinates(void);
};

#endif