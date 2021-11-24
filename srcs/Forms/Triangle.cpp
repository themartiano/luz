#include "Forms/Triangle.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Triangle with default values
Triangle::Triangle(void)
{
    this->_vertex1 = Vector3(0.0, 1.0, 0.0);
    this->_vertex2 = Vector3(-1.0, 0.0, 0.0);
    this->_vertex3 = Vector3(1.0, 0.0, 0.0);
    this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
}

// Constructs the Triangle with custom values
Triangle::Triangle(Vector3 vertex1, Vector3 vertex2, Vector3 vertex3, Material material)
{
    this->_vertex1 = vertex1;
    this->_vertex2 = vertex2;
    this->_vertex3 = vertex3;
    this->_material = material;
}

// Calculates if the Triangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Triangle::hit(Ray& ray, double t_max) const
{
    Vector3 v1 = this->_vertex2 - this->_vertex1;
    Vector3 v2 = this->_vertex3 - this->_vertex1;
    Vector3 n = cross(v1, v2);

    // Checks if the Triangle and the Ray are parallel
    double nDotRayDirection = dot(n, ray.getDirection());
    if (fabs(nDotRayDirection) < T_MIN)
    {
        return (false);
    }

    double d = dot(n, this->_vertex1);

    double t = (dot(n, ray.getOrigin()) + d) / nDotRayDirection;

    if (t < T_MIN || t > t_max)
    {
        return (false);
    }

    Vector3 p = ray.getOrigin() + t * ray.getDirection();

    Vector3 edge1 = this->_vertex2 - this->_vertex1;
    Vector3 vp1 = p - this->_vertex1;
    Vector3 c = cross(edge1, vp1);
    if (dot(n, c) < T_MIN)
    {
        return (false);
    }

    Vector3 edge2 = this->_vertex3 - this->_vertex2;
    Vector3 vp2 = p - this->_vertex2;
    c = cross(edge2, vp2);
    if (dot(n, c) < T_MIN)
    {
        return (false);
    }

    Vector3 edge3 = this->_vertex1 - this->_vertex3;
    Vector3 vp3 = p - this->_vertex3;
    c = cross(edge3, vp3);
    if (dot(n, c) < T_MIN)
    {
        return (false);
    }

    ray.hitRecord.t0 = t;
    ray.hitRecord.normal = normalize(n);
    ray.hitRecord.material = this->_material;
    ray.hitRecord.position = ray.pointAtRay(t);

    return (true);
}

// bool    Triangle::hit(Ray& ray, double t_max) const
// {
// 	Vector3 v1 = this->_vertex2 - this->_vertex1;
// 	Vector3 v2 = this->_vertex3 - this->_vertex1;
// 	double d = dot(v1, cross(ray.getDirection(), v2));
// 	if (fabs(d) < T_MIN)
// 		return (false);
// 	Vector3 t = ray.getOrigin() - this->_vertex1;
// 	double u = dot(t, cross(ray.getDirection(), v2)) * (1.0f / d);
// 	if (u < 0.0f || u > 1.0f || fabs(t.getZ()) > t_max)
// 		return (false);
// 	if (dot(ray.getDirection(), cross(t, v1)) * (1.0f / d) < 0.0f || u
// 		+ (dot(ray.getDirection(), cross(t, v1)) * (1.0f / d)) > 1.0f)
// 		return (false);

//     ray.hitRecord.t0 = dot(v2, cross(t, v1)) * (1.0f / d);
//     ray.hitRecord.normal = normalize(cross(v1, v2));
//     ray.hitRecord.material = this->_material;
//     ray.hitRecord.position = ray.pointAtRay(dot(v2, cross(t, v1)) * (1.0f / d));

// 	return (true);
// }

// Creates an AABB / bounding box for this Triangle
bool    Triangle::createBoundingBox(AABB& outputBoundingBox) const
{
    Vector3 minimum = this->_vertex1;
    Vector3 maximum = this->_vertex1;

    std::vector<Vector3> vectors;
    vectors.push_back(this->_vertex2);
    vectors.push_back(this->_vertex3);

    for (Vector3 vector : vectors)
    {
        if (vector.getX() < minimum.getX())
        {
            minimum.setX(vector.getX());
        }
        if (vector.getY() < minimum.getY())
        {
            minimum.setY(vector.getY());
        }
        if (vector.getZ() < minimum.getZ())
        {
            minimum.setZ(vector.getZ());
        }

        if (vector.getX() > maximum.getX())
        {
            maximum.setX(vector.getX());
        }
        if (vector.getY() > maximum.getY())
        {
            maximum.setY(vector.getY());
        }
        if (vector.getZ() > maximum.getZ())
        {
            maximum.setZ(vector.getZ());
        }
    }

    minimum.setZ(minimum.getZ() - T_MIN);
    maximum.setZ(maximum.getZ() + T_MIN);

    outputBoundingBox = AABB(minimum, maximum);

    return (true);
}
