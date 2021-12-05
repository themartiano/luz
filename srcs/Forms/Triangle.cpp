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
    this->_vertex0 = Vector3(0.0, 1.0, 0.0);
    this->_vertex1 = Vector3(-1.0, 0.0, 0.0);
    this->_vertex2 = Vector3(1.0, 0.0, 0.0);
    this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
}

// Constructs the Triangle with custom values
Triangle::Triangle(Vector3 vertex0, Vector3 vertex1, Vector3 vertex2, Material material)
{
    this->_vertex0 = vertex0;
    this->_vertex1 = vertex1;
    this->_vertex2 = vertex2;
    this->_material = material;
}

// Sets the Triangle's Vertex-0
void    Triangle::setVertex0(Vector3 vertex0)
{
    this->_vertex0 = vertex0;
}

// Sets the Triangle's Vertex-1
void    Triangle::setVertex1(Vector3 vertex1)
{
    this->_vertex1 = vertex1;
}

// Sets the Triangle's Vertex-2
void    Triangle::setVertex2(Vector3 vertex2)
{
    this->_vertex2 = vertex2;
}

// Sets the Triangle's Material
void    Triangle::setMaterial(Material material)
{
    this->_material = material;
}

// Calculates if the Triangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Triangle::hit(Ray& ray, double t_max) const
{
    Vector3 v1 = this->_vertex1 - this->_vertex0;
    Vector3 v2 = this->_vertex2 - this->_vertex0;
    Vector3 p = Utilities::cross(ray.getDirection(), v2);
    double  det = Utilities::dot(v1, p);

    // If det is near 0, they're parallel. If it's negative, the triangle is backfacing the camera.
    if (fabs(det) < T_MIN)
    {
        return (false);
    }

    double invDet = 1.0 / det;

    Vector3 tVec = ray.getOrigin() - this->_vertex0;
    double u = Utilities::dot(tVec, p) * invDet;
    if (u < 0.0 || u > 1.0)
    {
        return (false);
    }

    Vector3 q = Utilities::cross(tVec, v1);
    double v = Utilities::dot(ray.getDirection(), q) * invDet; // double v = Utilities::dot(ray.getDirection() * -1.0, q) * invDet; //////////// alguns triângulos ficam invertidos pois a direção do raio em relação à orientação do triângulo result em um valor negativo no / do DOT product. AKA normal invertido, etc. https://stackoverflow.com/a/40619957/11578778
    if (v < 0.0 || u + v > 1.0)
    {
        return (false);
    }

    double t = Utilities::dot(v2, q) * invDet;
    if (t > t_max || t < T_MIN)
    {
        return (false);
    }

    ray.hitRecord.t0 = t;
    ray.hitRecord.normal = Utilities::cross(v1, v2);
    ray.hitRecord.material = this->_material;
    ray.hitRecord.position = ray.pointAtRay(ray.hitRecord.t0);

    return (true);
}

//bool    Triangle::hit(Ray& ray, double t_max) const
// {
//     Vector3 v1 = this->_vertex2 - this->_vertex1;
//     Vector3 v2 = this->_vertex3 - this->_vertex1;
//     Vector3 n = Utilities::cross(v1, v2);

//     // Checks if the Triangle and the Ray are parallel
//     double nDotRayDirection = Utilities::dot(n, ray.getDirection());
//     if (fabs(nDotRayDirection) < T_MIN)
//     {
//         return (false);
//     }

//     double d = Utilities::dot(n, this->_vertex1);

//     double t = (Utilities::dot(n, ray.getOrigin()) + d) / nDotRayDirection;

//     if (t < T_MIN || t > t_max)
//     {
//         return (false);
//     }

//     Vector3 p = ray.getOrigin() + t * ray.getDirection();


//     Vector3 edge1 = this->_vertex2 - this->_vertex1;
//     Vector3 vp1 = p - this->_vertex1;
//     Vector3 c = Utilities::cross(edge1, vp1);
//     if (Utilities::dot(n, c) < T_MIN)
//     {
//         return (false);
//     }

//     Vector3 edge2 = this->_vertex3 - this->_vertex2;
//     Vector3 vp2 = p - this->_vertex2;
//     c = Utilities::cross(edge2, vp2);
//     if (Utilities::dot(n, c) < T_MIN)
//     {
//         return (false);
//     }

//     Vector3 edge3 = this->_vertex1 - this->_vertex3;
//     Vector3 vp3 = p - this->_vertex3;
//     c = Utilities::cross(edge3, vp3);
//     if (Utilities::dot(n, c) < T_MIN)
//     {

//         return (false);
//     }

//     ray.hitRecord.t0 = t;
//     ray.hitRecord.normal = Utilities::normalize(n);
//     ray.hitRecord.material = this->_material;
//     ray.hitRecord.position = ray.pointAtRay(t);

//     return (true);
// }

// bool    Triangle::hit(Ray& ray, double t_max) const
// {
// 	Vector3 v1 = this->_vertex2 - this->_vertex1;
// 	Vector3 v2 = this->_vertex3 - this->_vertex1;
// 	double d = Utilities::dot(v1, Utilities::cross(ray.getDirection(), v2));
// 	if (fabs(d) < T_MIN)
//     {
// 		return (false);
//     }

// 	Vector3 t = ray.getOrigin() - this->_vertex1;
// 	double u = Utilities::dot(t, Utilities::cross(ray.getDirection(), v2)) / d;
// 	if (u < 0.0f || u > 1.0f || fabs(t.getZ()) > t_max)
//     {
// 		return (false);
//     }

// 	if (Utilities::dot(ray.getDirection(), Utilities::cross(t, v1)) / d < 0.0f || u + (Utilities::dot(ray.getDirection(), Utilities::cross(t, v1)) / d) > 1.0f)
//     {

// 		return (false);
//     }

//     ray.hitRecord.t0 = Utilities::dot(v2, Utilities::cross(t, v1)) / d;
//     ray.hitRecord.normal = Utilities::normalize(Utilities::cross(v1, v2));
//     ray.hitRecord.material = this->_material;
//     ray.hitRecord.position = ray.pointAtRay(Utilities::dot(v2, Utilities::cross(t, v1)) / d);

// 	return (true);
// }

// Creates an AABB / bounding box for this Triangle
bool    Triangle::createBoundingBox(AABB& outputBoundingBox) const
{
    Vector3 minimum = Vector3(0.0, 0.0, 0.0);
    Vector3 maximum = Vector3(0.0, 0.0, 0.0);

    std::vector<Vector3> vectors;
    vectors.push_back(this->_vertex0);
    vectors.push_back(this->_vertex1);
    vectors.push_back(this->_vertex2);

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
