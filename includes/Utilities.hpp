#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include "Vector3.hpp"
#include "Scene.hpp"
#include <string>

class   Utilities
{
    public:
        static double   dot(Vector3 vector1, Vector3 vector2);
        static Vector3  cross(Vector3 vector1, Vector3 vector2);
        static double   vectorLengthNoSQRT(Vector3 vector);
        static double   vectorLength(Vector3 vector);
        static Vector3  normalize(Vector3 vector);
        static Vector3  randomPointInsideUnitSphere(void);
        static Vector3  randomPointInsideUnitDisk(void);
        static Vector3  reflect(Vector3 vector, Vector3 normal);
        static bool     refract(Vector3 vector, Vector3 normal, double refractiveIndex, Vector3& refractedVector);
        static double   schlick(double cosine, double refractiveIndex);
        static std::string  pluralOrSingular(int number);
        static void     setDoubleRange(double& flt, double min, double max);
        static bool     createMainBoundingBox(Scene& scene, AABB& newBoundingBox);
        static AABB     mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2);
        static void     toLower(std::string& str);
        static double   scatteringPDF(Ray& ray, Ray& scatteredRay);
        static Vector3  randomCosineDirection(void);
};

inline double	randomDouble(void)
{
    return (rand() / (RAND_MAX + 1.0));
}

inline double	randomDouble(double min, double max)
{
    return (min + (max - min) * randomDouble());
}

inline int	randomInt(int min, int max)
{
    return (static_cast<int>(randomDouble(min, max + 1)));
}

#endif