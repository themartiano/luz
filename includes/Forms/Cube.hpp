#ifndef CUBE_HPP
#define CUBE_HPP

#include "Hittable.hpp"
#include "Rectangle.hpp"
#include "Transform.hpp"
#include "Material.hpp"
#include <vector>

class   Cube : public Hittable
{
    public:
        Cube(void);
        Cube(Transform transform, Material material, double width, double height, double depth);
        void    setTransform(Transform transform);
        void    setMaterial(Material material);
        void    setWidth(double width);
        void    setHeight(double height);
        void    setDepth(double depth);
		virtual bool    hit(Ray& ray, double t_max) const override;
		virtual bool    createBoundingBox(AABB& outputBoundingBox) const override;

    private:
        Transform   _transform;
        Material    _material;
        double      _width;
        double      _height;
        double      _depth;
        std::vector<Rectangle>  _faces;
        void    _generateFaces(void);
};

#endif