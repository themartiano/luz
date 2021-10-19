#ifndef RAY_HPP
# define RAY_HPP

#include "Scene.hpp"
#include "Vector3.hpp"
#include <cmath>

class   Ray
{
    public:
        Ray(void);
        Ray(Vector3 origin, Vector3 direction, Scene scene, int x, int y);
        Vector3 pointAtRay(float t);
        Vector3 getOrigin(void) const;
        Vector3 getDirection(void) const;

    private:
        Vector3 _origin;
        Vector3 _direction;
};

#endif