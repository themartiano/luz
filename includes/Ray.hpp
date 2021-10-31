#ifndef RAY_HPP
#define RAY_HPP

#include "Vector3.hpp"
#include "Material.hpp"

struct HitRecord
{
    double      t0 = 0.0;
    double      t1 = 0.0;
    Vector3     position = Vector3(0.0, 0.0, 0.0);
    Vector3     normal = Vector3(0.0, 0.0, 0.0);
    Material    material = Material();
};


class   Ray
{
    public:
        Ray(void);
        Ray(Vector3 origin, Vector3 direction);
        Vector3 pointAtRay(double t);
        Vector3 getOrigin(void) const;
        void    setOrigin(Vector3 origin);
        Vector3 getDirection(void) const;
        void    setDirection(Vector3 direction);
        struct HitRecord    hitRecord;

    private:
        Vector3 _origin;
        Vector3 _direction;

};

#endif