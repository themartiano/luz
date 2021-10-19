#ifndef RAY_HPP
# define RAY_HPP

#include "Scene.hpp"
#include "Vector3.hpp"
#include <cmath>

struct HitRecord
{
    float   t;
    Vector3 position;
    Vector3 normal;
};


class   Ray
{
    public:
        Ray(void);
        Ray(Vector3 origin, Vector3 direction);
        Vector3 pointAtRay(float t);
        Vector3 getOrigin(void) const;
        Vector3 getDirection(void) const;
        struct HitRecord    hitRecord;

    private:
        Vector3 _origin;
        Vector3 _direction;

};

#endif