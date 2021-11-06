#ifndef ATMOSPHERE_HPP
#define ATMOSPHERE_HPP

#include "Vector3.hpp"
#include "Ray.hpp"
#include "Forms/Sphere.hpp"

class   Atmosphere
{
    public:
        Atmosphere(void);
        Atmosphere(double sunAngle, double earthRadius, double atmosphereRadius, double hR, double hM, int samples, int lightSamples);
        static const Vector3 betaR;
        static const Vector3 betaM;
        double  getEarthRadius(void) const;
        Color   computeIncidentLight(Ray& ray, double t_max);

    private:
        Vector3 _sunDirection;
        double  _earthRadius;
        double  _atmosphereRadius;
        double  _hR; // Thickness of the atmosphere if density was uniform
        double  _hM; // The same as above but for Mie Scattering
        int     _samples; // Samples per ray
        int     _lightSamples; // Number of samples per ray sample
};

bool planetaryHit(double radius, Ray& ray);

#endif