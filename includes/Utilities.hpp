#pragma once

#include "Vector3.hpp"
#include "Scene/Scene.hpp"
#include "Hittables/Hittable.hpp"
#include <string>

namespace Utilities
{
	double  dot(Vector3 vector1, Vector3 vector2);
	Vector3 cross(Vector3 vector1, Vector3 vector2);
	double  vectorLengthSquared(Vector3 vector);
	double  vectorLength(Vector3 vector);
	Vector3 normalize(Vector3 vector);
	Vector3 reflect(Vector3 vector, Vector3 normal);
	Vector3	refract(Vector3 vector, Vector3 normal, double refractiveIndex);
	double  schlick(double cosine, double refractiveIndex);
	std::string pluralOrSingular(int number);
	void	setDoubleRange(double& flt, double min, double max);
	bool	createMainBoundingBox(Scene& scene, AABB& newBoundingBox);
	AABB	mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2);
	AABB	mergeBoundingBoxes(std::vector<AABB> boundingBoxes);
	void	toLower(std::string& str);
	bool	stringEndsWith(std::string str, std::string ending);
}
