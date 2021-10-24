#include "HitUtils.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>

// Calculates if 'sphere' is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	hitSphere(Ray& ray, Sphere sphere, float t_max)
{
	Vector3 oc = ray.getOrigin() - sphere.getPosition();
	float a = dot(ray.getDirection(), ray.getDirection());
	float b = dot(oc, ray.getDirection());
	float c = dot(oc, oc) - (sphere.getRadius() * sphere.getRadius());
	float discriminant = (b * b) - (a * c);
	if (discriminant > 0.0f)
    {
        float temp = (-b - sqrt((b * b) - (a * c))) / a;
        if (temp < t_max && temp > T_MIN)
        {
            ray.hitRecord.t = temp;
            ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t);
            ray.hitRecord.normal = (ray.hitRecord.position - sphere.getPosition()) / sphere.getRadius();
            return (true);
        }
        temp = (-b + sqrt((b * b) - (a * c))) / a;
        if (temp < t_max && temp > T_MIN)
        {
            ray.hitRecord.t = temp;
            ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t);
            ray.hitRecord.normal = (ray.hitRecord.position - sphere.getPosition()) / sphere.getRadius();
            return (true);
        }
    }
    return (false);
}
