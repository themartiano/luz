#ifndef RAY_HPP
# define RAY_HPP

#include "Vector3.hpp"

class   Ray
{
    public:
        Ray(void);
        Ray(Vector3 origin, Vector3 direction);
        Vector3 pointAtRay(float t);

    private:
        Vector3 _origin;
        Vector3 _direction;
};

#endif