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
    Vector3 p = cross(ray.getDirection(), v2);
    double  det = dot(v1, p);

    if (fabs(det) < T_MIN)
    {
        return (false);
    }

    double  invDet = 1.0 / det;

    Vector3 t = ray.getOrigin() - this->_vertex1;
    double u = dot(t, p) * invDet;
    if (u < 0.0 || u > 1.0 || fabs(t.getZ()) > t_max)
    {
        return (false);
    }

    Vector3 q = cross(t, v1);
    double v = dot(ray.getDirection(), q) * invDet;
    if (v < 0.0 || u + v > 1.0)
    {
        return (false);
    }

    ray.hitRecord.t0 = dot(v2, q) * invDet;
    ray.hitRecord.normal = cross(v1, v2);
    ray.hitRecord.material = this->_material;
    ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t0);

    return (true);
}

// bool    Triangle::hit(Ray& ray, double t_max) const
// {
// 	Vector3 v1 = this->_vertex2 - this->_vertex1;
// 	Vector3 v2 = this->_vertex3 - this->_vertex1;
// 	double d = dot(v1, cross(ray.getDirection(), v2));
// 	if (fabs(d) < T_MIN)
//     {
// 		return (false);
//     }

// 	Vector3 t = ray.getOrigin() - this->_vertex1;
// 	double u = dot(t, cross(ray.getDirection(), v2)) / d;
// 	if (u < 0.0f || u > 1.0f || fabs(t.getZ()) > t_max)
//     {
// 		return (false);
//     }

// 	if (dot(ray.getDirection(), cross(t, v1)) / d < 0.0f || u + (dot(ray.getDirection(), cross(t, v1)) / d) > 1.0f)
//     {

// 		return (false);
//     }

//     ray.hitRecord.t0 = dot(v2, cross(t, v1)) / d;
//     ray.hitRecord.normal = normalize(cross(v1, v2));
//     ray.hitRecord.material = this->_material;
//     ray.hitRecord.position = ray.pointAtRay(dot(v2, cross(t, v1)) / d);

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
