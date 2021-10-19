#include "HitUtils.hpp"

bool	hitSphere(Ray ray, Sphere sphere)
{
	Vector3 oc = ray.getOrigin() - sphere.getTransform().getPosition();
	float a = dot(ray.getDirection(), ray.getDirection());
	float b = dot(oc, ray.getDirection());
	float c = dot(oc, oc) - (sphere.getRadius() * sphere.getRadius());
	float discriminant = (b * b) - (a * c);
	return (discriminant > 0.0f);
}