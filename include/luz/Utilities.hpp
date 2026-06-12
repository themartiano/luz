#pragma once

#include "Color.hpp"
#include "Vector3.hpp"
#include "Scene/Scene.hpp"
#include "Hittables/Hittable.hpp"
#include <cmath>
#include <string>

namespace Utilities
{
	inline double  dot(const Vector3& vector1, const Vector3& vector2)
	{
		return (
			(vector1.getX() * vector2.getX()) +
			(vector1.getY() * vector2.getY()) +
			(vector1.getZ() * vector2.getZ())
		);
	}

	inline Vector3 cross(const Vector3& vector1, const Vector3& vector2)
	{
		return (Vector3(
			(vector1.getY() * vector2.getZ()) - (vector1.getZ() * vector2.getY()),
			(vector1.getZ() * vector2.getX()) - (vector1.getX() * vector2.getZ()),
			(vector1.getX() * vector2.getY()) - (vector1.getY() * vector2.getX()))
		);
	}

	inline double  vectorLengthSquared(const Vector3& vector)
	{
		return (
			(vector.getX() * vector.getX()) +
			(vector.getY() * vector.getY()) +
			(vector.getZ() * vector.getZ())
		);
	}

	inline double  vectorLength(const Vector3& vector)
	{
		return (std::sqrt(vectorLengthSquared(vector)));
	}

	inline Vector3 normalize(const Vector3& vector)
	{
		return (vector / vectorLength(vector));
	}

	inline Vector3 reflect(const Vector3& vector, const Vector3& normal)
	{
		return (vector - 2.0 * dot(vector, normal) * normal);
	}

	inline Vector3	refract(const Vector3& vector, const Vector3& normal, double refractiveIndex)
	{
		const double	cosTheta = std::fmin(dot(vector * -1.0, normal), 1.0);
		const Vector3	perpendicularRefract = refractiveIndex * (vector + cosTheta * normal);
		const Vector3	parallelRefract = -std::sqrt(std::fabs(1.0 - vectorLengthSquared(perpendicularRefract))) * normal;

		return (perpendicularRefract + parallelRefract);
	}

	inline double  schlick(double cosine, double refractiveIndex)
	{
		double r0 = (1.0 - refractiveIndex) / (1.0 + refractiveIndex);
		r0 *= r0;

		return (r0 + (1.0 - r0) * std::pow((1.0 - cosine), 5));
	}

	std::string pluralOrSingular(int number);
	void	setDoubleRange(double& flt, double min, double max);
	bool	createMainBoundingBox(Scene& scene, AABB& newBoundingBox);
	AABB	mergeBoundingBoxes(AABB boundingBox1, AABB boundingBox2);
	AABB	mergeBoundingBoxes(std::vector<AABB> boundingBoxes);
	void	toLower(std::string& str);
	bool	stringEndsWith(std::string str, std::string ending);
	std::string	terminalFilePath(const std::string& filePath);
	double	luminance(const Color& color);
	Color	filmicToneMap(const Color& color);
	Color	reinhardJodie(const Color& color);
}
