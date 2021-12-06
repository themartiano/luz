#ifndef ATMOSPHERE_HPP
#define ATMOSPHERE_HPP

#include "Vector3.hpp"
#include "Ray.hpp"
#include "Forms/Sphere.hpp"

class   Atmosphere
{
    public:
        Atmosphere(void);
        Atmosphere(double sunAngle, double earthRadius, double atmosphereRadius, double hR, double hM, int samples, int lightSamples, double starsBrightness);
        static const Vector3 betaR;
        static const Vector3 betaM;
        double  getEarthRadius(void) const;
        double  getAtmosphereRadius(void) const;
        double  getStarsBrightness(void) const;
        Color   computeIncidentLight(Ray& ray, double t_max);
        double  getSunAngle(void) const;
        void    setSunAngle(double newAngle);
        void    setEarthRadius(double earthRadius);
        void    setAtmosphereRadius(double atmosphereRadius);
        void    setHR(double hR);
        void    setHM(double hM);
        void    setSamples(int samples);
        void    setLightSamples(int lightSamples);
        void    setStarsBrightness(double starsBrightness);

    private:
        Vector3 _sunDirection;
        double  _sunAngle;
        double  _earthRadius;
        double  _atmosphereRadius;
        double  _hR; // Thickness of the atmosphere if density was uniform
        double  _hM; // The same as above but for Mie Scattering
        int     _samples; // Samples per ray
        int     _lightSamples; // Number of samples per ray sample
        double  _starsBrightness;
        void    updateSunDirectionVector(void);
};

bool planetaryHit(double radius, Ray& ray);

#endif