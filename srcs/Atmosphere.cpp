#include "Atmosphere.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "SystemSpecifics.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Atmosphere with default values
Atmosphere::Atmosphere(void)
{
	this->_sunAngle = -0.4;
	updateSunDirectionVector();
	this->_earthRadius = D_EARTH_RADIUS;
	this->_atmosphereRadius = D_ATMOSPHERE_RADIUS;
	this->_hR = D_HR;
	this->_hM = D_HM;
	this->_samples = 16;
	this->_lightSamples = 8;
	this->_starsBrightness = 0.5;
}

// Constructs the Atmosphere with custom values
Atmosphere::Atmosphere(double sunAngle, double earthRadius, double atmosphereRadius, double hR, double hM, int samples, int lightSamples, double starsBrightness)
{
	this->_sunAngle = sunAngle;
	updateSunDirectionVector();
	this->_earthRadius = earthRadius;
	this->_atmosphereRadius = atmosphereRadius;
	this->_hR = hR;
	this->_hM = hM;
	this->_samples = samples;
	this->_lightSamples = lightSamples;
	this->_starsBrightness = starsBrightness;
}

const Vector3 Atmosphere::betaR(3.8e-6, 13.5e-6, 33.1e-6);
const Vector3 Atmosphere::betaM(21e-6, 21e-6, 21e-6);

// Returns the Earth Radius
double  Atmosphere::getEarthRadius(void) const
{
	return (this->_earthRadius);
}

//Returns the Atmosphere Radius
double  Atmosphere::getAtmosphereRadius(void) const
{
	return (this->_atmosphereRadius);
}

// Returns the Earth Radius
double  Atmosphere::getStarsBrightness(void) const
{
	return (this->_starsBrightness);
}

//Returns the Sun Angle (double)
double  Atmosphere::getSunAngle(void) const
{
	return (this->_sunAngle);
}

//Sets the Sun Angle and updates internal variables
void	Atmosphere::setSunAngle(double newAngle)
{
	this->_sunAngle = newAngle;
	updateSunDirectionVector();
}

// Sets the EarthRadius
void	Atmosphere::setEarthRadius(double earthRadius)
{
	this->_earthRadius = earthRadius;
}

// Sets the AtmosphereRadius
void	Atmosphere::setAtmosphereRadius(double atmosphereRadius)
{
	this->_atmosphereRadius = atmosphereRadius;
}

// Sets the HR value
void	Atmosphere::setHR(double hR)
{
	this->_hR = hR;
}

// Sets the HM value
void	Atmosphere::setHM(double hM)
{
	this->_hM = hM;
}

// Sets the Sample count
void	Atmosphere::setSamples(int samples)
{
	this->_samples = samples;
}

// Sets the Light Sample count
void	Atmosphere::setLightSamples(int lightSamples)
{
	this->_lightSamples = lightSamples;
}

// Sets the Stars Brightness
void	Atmosphere::setStarsBrightness(double starsBrightness)
{
	this->_starsBrightness = starsBrightness;
}

//Updates Sun Direction (Vector3) using Sun Angle (double)
void	Atmosphere::updateSunDirectionVector(void)
{
	double angle = D_PI * this->_sunAngle;
	this->_sunDirection = Vector3(0.0, std::cos(angle), -std::sin(angle));
}

// (Sphere) Hit function for planets and atmospheres (sets both t0 and t1 on the Hit Record). Returns true if hit occurs, false otherwise
bool planetaryHit(double radius, Ray& ray, HitRecord& hitRecord)
{
	double a = Utilities::dot(ray.getDirection(), ray.getDirection());
	double b = 2.0 * Utilities::dot(ray.getDirection(), ray.getOrigin());
	double c = Utilities::dot(ray.getOrigin(), ray.getOrigin()) - radius * radius;

	if (b == 0.0)
	{
		if (a == 0.0)
		{
			return (false);
		}

		hitRecord.t0 = 0.0;
		hitRecord.t1 = std::sqrt(-c / a);

		if (hitRecord.t0 > hitRecord.t1)
		{
			std::swap(hitRecord.t0, hitRecord.t1);
		}

		return (true);
	}
	double discriminant = b * b - 4 * a * c;

	if (discriminant < 0.0)
	{
		return (false);
	}

	double q = (b < 0.0) ? -0.5 * (b - std::sqrt(discriminant)) : -0.5 * (b + std::sqrt(discriminant));
	hitRecord.t0 = q / a;
	hitRecord.t1 = c / q;

	if (hitRecord.t0 > hitRecord.t1)
	{
		std::swap(hitRecord.t0, hitRecord.t1);
	}

	return (true);
}

// Returns the sky color for 'ray'
Color   Atmosphere::computeIncidentLight(Ray& ray, HitRecord& hitRecord, double t_max)
{
	double  t_min = T_MIN;

	if (!planetaryHit(this->_atmosphereRadius, ray, hitRecord) || hitRecord.t1 < 0.0)
	{
		return (Color(0.0, 0.0, 0.0));
	}
	if (hitRecord.t0 > T_MIN && hitRecord.t0 > 0.0)
	{
		t_min = hitRecord.t0;
	}
	if (hitRecord.t1 < t_max)
	{
		t_max = hitRecord.t1;
	}

	double  segmentLength = (t_max - t_min) / this->_samples;
	double  tCurrent = t_min;
	Vector3 sumR(0.0, 0.0, 0.0);
	Vector3 sumM(0.0, 0.0, 0.0);
	double  transmittanceR = 0.0;
	double  transmittanceM = 0.0;
	double  mu = Utilities::dot(ray.getDirection(), this->_sunDirection);
	static double   g = 0.76;
	static double   phaseRStatic = 3.0 / (16.0 * D_PI);
	static double   phaseMStatic = 3.0 / (8.0 * D_PI);
	double  phaseR = phaseRStatic * (1.0 + mu * mu);
	double  phaseM = phaseMStatic * ((1.0 - g * g) * (1.0 + mu * mu) / ((2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * mu, 1.5)));

	for (int i = 0; i < this->_samples; i++)
	{
		Vector3 samplePosition = ray.getOrigin() + (tCurrent + segmentLength * 0.5) * ray.getDirection();
		double  height = Utilities::vectorLength(samplePosition) - this->_earthRadius;

		double  hR = exp(-height / this->_hR) * segmentLength;
		double  hM = exp(-height / this->_hM) * segmentLength;
		transmittanceR += hR;
		transmittanceM += hM;
		Ray ray2(samplePosition, this->_sunDirection);
		HitRecord hitRecord2;
		planetaryHit(this->_atmosphereRadius, ray2, hitRecord2);
		double  segmentLengthLight = hitRecord2.t1 / this->_lightSamples;
		double  tCurrentLight = 0.0;
		double  transmittanceLightR = 0.0;
		double  transmittanceLightM = 0.0;

		int j;
		for (j = 0; j < this->_lightSamples; j++)
		{
			Vector3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5) * this->_sunDirection;
			double  heightLight = Utilities::vectorLength(samplePositionLight) - this->_earthRadius;
			if (heightLight < 0.0)
			{
				break;
			}
			transmittanceLightR += exp(-heightLight / this->_hR) * segmentLengthLight;
			transmittanceLightM += exp(-heightLight / this->_hM) * segmentLengthLight;
			tCurrentLight += segmentLengthLight;
		}

		if (j == this->_lightSamples)
		{
			Vector3 tau = betaR * (transmittanceR + transmittanceLightR) + betaM * 1.1 * (transmittanceM + transmittanceLightM);
			Vector3 attenuation(exp(-tau.getX()), exp(-tau.getY()), exp(-tau.getZ()));
			sumR += attenuation * hR;
			sumM += attenuation * hM;
		}

		tCurrent += segmentLength;
	}

	// Turns NaNs into zeros
	// if (phaseM != phaseM)
	// {
	//	 phaseM = 0.0;
	// }

	Vector3 result = (sumR * betaR * phaseR + sumM * betaM * phaseM) * 20.0;
	return (Color(result.getX(), result.getY(), result.getZ()));
}
