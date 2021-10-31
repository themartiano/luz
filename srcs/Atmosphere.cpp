#include "Atmosphere.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "SystemSpecifics.hpp"
#include <cmath>

Atmosphere::Atmosphere(void)
{
    // double angle = 64 / float(128) * M_PI * 0.6;
    // this->_sunDirection = Vector3(0.0, cos(angle), -sin(angle));

    this->_sunDirection = Vector3(0.0, 1.0, 0.0);

    this->_earthRadius = 6360e3;
    this->_atmosphereRadius = 6420e3;
    this->_hR = 7994.0;
    this->_hM = 1200.0;
}

const Vector3 Atmosphere::betaR(3.8e-6, 13.5e-6, 33.1e-6);
const Vector3 Atmosphere::betaM(21e-6, 21e-6, 21e-6);

Vector3 Atmosphere::getSunDirection(void) const
{
    return (this->_sunDirection);
}

double  Atmosphere::getEarthRadius(void) const
{
    return (this->_earthRadius);
}

double  Atmosphere::getAtmosphereRadius(void) const
{
    return (this->_atmosphereRadius);
}

double  Atmosphere::getHR(void) const
{
    return (this->_hR);
}

double  Atmosphere::getHM(void) const
{
    return (this->_hM);
}

bool hitAtmosphere(Sphere& atmosphere, Ray& ray)
{
    double a = dot(ray.getDirection(), ray.getDirection());
    double b = 2.0 * dot(ray.getDirection(), ray.getOrigin());
    double c = dot(ray.getOrigin(), ray.getOrigin()) - atmosphere.getRadius() * atmosphere.getRadius();

    if (b == 0.0)
    {
        if (a == 0.0)
        {
            return (false);
        }

        ray.hitRecord.t0 = 0.0;
        ray.hitRecord.t1 = std::sqrt(-c / a);

        if (ray.hitRecord.t0 > ray.hitRecord.t1)
        {
            std::swap(ray.hitRecord.t0, ray.hitRecord.t1);
        }

        return (true);
    }
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0.0)
    {
        return (false);
    }

    double q = (b < 0.0) ? -0.5 * (b - std::sqrt(discriminant)) : -0.5 * (b + std::sqrt(discriminant));
    ray.hitRecord.t0 = q / a;
    ray.hitRecord.t1 = c / q;

    if (ray.hitRecord.t0 > ray.hitRecord.t1)
    {
        std::swap(ray.hitRecord.t0, ray.hitRecord.t1);
    }

    return (true);
}
#include <iostream>
Color   Atmosphere::computeIncidentLight(Ray& ray, double t_max)
{
    double  t_min = T_MIN;

    Sphere  atmosphere(Vector3(0.0, 0.0, 0.0), Material(), this->_atmosphereRadius);

    if (!hitAtmosphere(atmosphere, ray) || ray.hitRecord.t1 < 0.0)
    {
        return (Color(0.0, 0.0, 0.0));
    }
    if (ray.hitRecord.t0 > T_MIN && ray.hitRecord.t0 > 0.0)
    {
        t_min = ray.hitRecord.t0;
    }
    if (ray.hitRecord.t1 < t_max)
    {
        t_max = ray.hitRecord.t1;
    }

    int numSamples = 8;
    int numSamplesLight = 4;

    double  segmentLength = (t_max - t_min) / numSamples;
    double  tCurrent = t_min;
    Vector3 sumR(0.0, 0.0, 0.0);
    Vector3 sumM(0.0, 0.0, 0.0);
    double  transmittanceR = 0.0;
    double  transmittanceM = 0.0;
    double  mu = dot(ray.getDirection(), this->_sunDirection);
    double  phaseR = 3.0 / (16.0 * M_PI) * (1.0 + mu * mu);
    double  g = 0.76;
    double  phaseM = 3.0 / (8.0 * M_PI) * ((1.0 - g * g) * (1.0 +  mu * mu)) / ((2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * mu, 1.5));
    for (int i = 0; i < numSamples; ++i)
    {
        Vector3 samplePosition = ray.getOrigin() + (tCurrent + segmentLength * 0.5) * ray.getDirection();
        double  height = vectorLength(samplePosition) - this->_earthRadius;

        double  hR = exp(-height / this->_hR) * segmentLength;
        double  hM = exp(-height / this->_hM) * segmentLength;
        transmittanceR += hR;
        transmittanceM += hM;
        Ray ray2(samplePosition, this->_sunDirection);
        Sphere atmosphere2(Vector3(0.0, 0.0, 0.0), Material(), this->_atmosphereRadius);
        hitAtmosphere(atmosphere2, ray2);
        double  segmentLengthLight = ray2.hitRecord.t1 / numSamplesLight;
        double  tCurrentLight = 0.0;
        double  transmittanceLightR = 0.0;
        double  transmittanceLightM = 0.0;
        int     j;
        //std::cout << this->_sunDirection.getX() << ", " << this->_sunDirection.getY() << ", " << this->_sunDirection.getZ() << std::endl;
        for (j = 0; j < numSamplesLight; ++j)
        {
            Vector3 samplePositionLight = samplePosition - (tCurrentLight + segmentLengthLight * 0.5) * this->_sunDirection;
            double  heightLight = vectorLength(samplePositionLight) + this->_earthRadius;
            if (heightLight < 0.0)
            {
                break;
            }
            transmittanceLightR += exp(-heightLight / this->_hR) * segmentLengthLight;
            transmittanceLightM += exp(-heightLight / this->_hM) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight)
        {
            Vector3 tau = betaR * (transmittanceR + transmittanceLightR) + betaM * 1.1 * (transmittanceM + transmittanceLightM);
            Vector3 attenuation(exp(-tau.getX()), exp(-tau.getY()), exp(-tau.getZ()));
            sumR += attenuation * hR;
            sumM += attenuation * hM;
        }
        tCurrent += segmentLength;
    }

    Vector3 result = (sumR * betaR * phaseR + sumM * betaM * phaseM) * 20.0;
    //std::cout << result.getX() << ", " << result.getY() << ", " << result.getZ() << std::endl;
    return (Color(result.getX(), result.getY(), result.getZ()));
}
