#ifndef ATMOSPHERE_HPP
#define ATMOSPHERE_HPP

#include "Vector3.hpp"
#include "Ray.hpp"

class   Atmosphere
{
    public:
        Atmosphere(void);
        static const Vector3 betaR;
        static const Vector3 betaM;
        Vector3 getSunDirection(void) const;
        double  getEarthRadius(void) const;
        double  getAtmosphereRadius(void) const;
        double  getHR(void) const;
        double  getHM(void) const;
        Color   computeIncidentLight(Ray& ray);

    private:
        Vector3 _sunDirection;
        double  _earthRadius;
        double  _atmosphereRadius;
        double  _hR; // Thickness of the atmosphere if density was uniform
        double  _hM; // The same as above but for Mie Scattering
};

#endif