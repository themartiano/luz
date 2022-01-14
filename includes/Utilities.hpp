#pragma once

#include "Vector3.hpp"
#include "Scene.hpp"
#include <string>

namespace Utilities
{
	double  dot(Vector3 vector1, Vector3 vector2);
	Vector3 cross(Vector3 vector1, Vector3 vector2);
	double  vectorLengthSquared(Vector3 vector);
	double  vectorLength(Vector3 vector);
	Vector3 normalize(Vector3 vector);
	Vector3 randomPointInsideUnitSphere(void);
	Vector3 randomPointInsideUnitDisk(void);
	Vector3 reflect(Vector3 vector, Vector3 normal);
	Vector3	refract(Vector3 vector, Vector3 normal, double refractiveIndex);
	double  schlick(double cosine, double refractiveIndex);
	std::string pluralOrSingular(int number);
	void	setDoubleRange(double& flt, double min, double max);
	bool	createMainBoundingBox(Scene& scene, AABB& newBoundingBox);
	AABB	mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2);
	void	toLower(std::string& str);
	double  scatteringPDF(const Ray& ray, const Ray& scatteredRay);
	Vector3 randomCosineDirection(void);
	bool	stringEndsWith(std::string str, std::string ending);
	double	randomDouble(void);
	double	randomDouble(double min, double max);
	int	randomInt(int min, int max);
}
