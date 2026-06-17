#include "Hittables/Rectangle.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include "Sampler.hpp"
#include <cmath>
#include <algorithm>

namespace
{
	bool	buildRectangleBasis(const Vector3& orientation, Vector3& normal, Vector3& widthAxis, Vector3& heightAxis)
	{
		if (Utilities::vectorLengthSquared(orientation) <= 0.0)
		{
			return (false);
		}

		normal = Utilities::normalize(orientation);

		if (std::fabs(normal.getY()) > 0.999)
		{
			widthAxis = Vector3(1.0, 0.0, 0.0);
			heightAxis = Vector3(0.0, 0.0, 1.0);
			return (true);
		}
		if (std::fabs(normal.getZ()) > 0.999)
		{
			widthAxis = Vector3(1.0, 0.0, 0.0);
			heightAxis = Vector3(0.0, 1.0, 0.0);
			return (true);
		}
		if (std::fabs(normal.getX()) > 0.999)
		{
			widthAxis = Vector3(0.0, 0.0, 1.0);
			heightAxis = Vector3(0.0, 1.0, 0.0);
			return (true);
		}

		Vector3 helper(0.0, 1.0, 0.0);
		if (std::fabs(Utilities::dot(helper, normal)) > 0.9)
		{
			helper = Vector3(1.0, 0.0, 0.0);
		}
		widthAxis = Utilities::normalize(Utilities::cross(helper, normal));
		heightAxis = Utilities::normalize(Utilities::cross(normal, widthAxis));

		return (true);
	}
}

/*
	Constructors
*/

// Constructs the Rectangle with default values
Rectangle::Rectangle(void)
{
	this->_transform = Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0));
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_width = 1.0;
	this->_height = 1.0;
}

Rectangle::Rectangle(const Rectangle& toCopy)
{
	this->_height = toCopy._height;
	this->_width = toCopy._width;
	this->_material = toCopy._material;
	this->_transform = toCopy._transform;
}

// Constructs the Rectangle with custom values
Rectangle::Rectangle(Transform transform, double width, double height, std::shared_ptr<Material> material)
{
	this->_transform = transform;
	this->_width = width;
	this->_height = height;
	this->_material = material;
}

// Sets the Rectangle's Transform
void	Rectangle::setTransform(Transform transform)
{
	this->_transform = transform;
}

// Returns the Rectangle's material
Material*	Rectangle::getMaterial(void) const
{
	return (this->_material.get());
}

// Sets the Rectangle's Material
void	Rectangle::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Sets the Rectangle's Width
void	Rectangle::setWidth(double width)
{
	this->_width = width;
}

// Sets the Rectangle's Height
void	Rectangle::setHeight(double height)
{
	this->_height = height;
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Rectangle::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Vector3 normal;
	Vector3 widthAxis;
	Vector3 heightAxis;

	if (!buildRectangleBasis(this->_transform.getOrientation(), normal, widthAxis, heightAxis))
	{
		return (false);
	}

	double a = Utilities::dot(ray.getOrigin() - this->_transform.getPosition(), normal);
	double b = Utilities::dot(ray.getDirection(), normal);
	if (b == 0.0 || (a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0))
	{
		return (false);
	}

	double t = -a / b;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	Vector3 d = ray.pointAtRay(t) - this->_transform.getPosition();
	const double widthDistance = Utilities::dot(d, widthAxis);
	const double heightDistance = Utilities::dot(d, heightAxis);
	if (std::fabs(widthDistance) > (this->_width / 2.0) || std::fabs(heightDistance) > (this->_height / 2.0))
	{
		return (false);
	}

	hitRecord.t0 = t;
	hitRecord.setFaceNormal(ray, normal);
	hitRecord.material = this->_material.get();
	hitRecord.position = ray.pointAtRay(t);

	return (true);
}

bool	Rectangle::hitAny(Ray& ray, double t_min, double t_max) const
{
	Vector3 normal;
	Vector3 widthAxis;
	Vector3 heightAxis;

	if (!buildRectangleBasis(this->_transform.getOrientation(), normal, widthAxis, heightAxis))
	{
		return (false);
	}

	double a = Utilities::dot(ray.getOrigin() - this->_transform.getPosition(), normal);
	double b = Utilities::dot(ray.getDirection(), normal);
	if (b == 0.0 || (a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0))
	{
		return (false);
	}

	double t = -a / b;
	if (t > t_max || t < t_min)
	{
		return (false);
	}

	Vector3 d = ray.pointAtRay(t) - this->_transform.getPosition();
	const double widthDistance = Utilities::dot(d, widthAxis);
	const double heightDistance = Utilities::dot(d, heightAxis);
	return (
		std::fabs(widthDistance) <= (this->_width / 2.0)
		&& std::fabs(heightDistance) <= (this->_height / 2.0)
	);
}

// Creates an AABB / bounding box for this Rectangle
bool	Rectangle::createBoundingBox(AABB& outputBoundingBox) const
{
	const Vector3 position = this->_transform.getPosition();
	Vector3 normal;
	Vector3 widthAxis;
	Vector3 heightAxis;
	const double halfWidth = this->_width / 2.0;
	const double halfHeight = this->_height / 2.0;

	if (!buildRectangleBasis(this->_transform.getOrientation(), normal, widthAxis, heightAxis))
	{
		return (false);
	}

	if (std::fabs(normal.getY()) > 0.999)
	{
		outputBoundingBox = AABB(
			Vector3(position.getX() - halfWidth, position.getY() - T_MIN, position.getZ() - halfHeight),
			Vector3(position.getX() + halfWidth, position.getY() + T_MIN, position.getZ() + halfHeight)
		);
		return (true);
	}
	if (std::fabs(normal.getZ()) > 0.999)
	{
		outputBoundingBox = AABB(
			Vector3(position.getX() - halfWidth, position.getY() - halfHeight, position.getZ() - T_MIN),
			Vector3(position.getX() + halfWidth, position.getY() + halfHeight, position.getZ() + T_MIN)
		);
		return (true);
	}
	if (std::fabs(normal.getX()) > 0.999)
	{
		outputBoundingBox = AABB(
			Vector3(position.getX() - T_MIN, position.getY() - halfHeight, position.getZ() - halfWidth),
			Vector3(position.getX() + T_MIN, position.getY() + halfHeight, position.getZ() + halfWidth)
		);
		return (true);
	}

	const Vector3 corner0 = position + (widthAxis * halfWidth) + (heightAxis * halfHeight);
	const Vector3 corner1 = position + (widthAxis * halfWidth) - (heightAxis * halfHeight);
	const Vector3 corner2 = position - (widthAxis * halfWidth) + (heightAxis * halfHeight);
	const Vector3 corner3 = position - (widthAxis * halfWidth) - (heightAxis * halfHeight);

	Vector3 minimum(
		std::min(std::min(corner0.getX(), corner1.getX()), std::min(corner2.getX(), corner3.getX())) - T_MIN,
		std::min(std::min(corner0.getY(), corner1.getY()), std::min(corner2.getY(), corner3.getY())) - T_MIN,
		std::min(std::min(corner0.getZ(), corner1.getZ()), std::min(corner2.getZ(), corner3.getZ())) - T_MIN
	);
	Vector3 maximum(
		std::max(std::max(corner0.getX(), corner1.getX()), std::max(corner2.getX(), corner3.getX())) + T_MIN,
		std::max(std::max(corner0.getY(), corner1.getY()), std::max(corner2.getY(), corner3.getY())) + T_MIN,
		std::max(std::max(corner0.getZ(), corner1.getZ()), std::max(corner2.getZ(), corner3.getZ())) + T_MIN
	);

	outputBoundingBox = AABB(minimum, maximum);

	return (true);
}

double  Rectangle::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	Ray ray(origin, vec);
	HitRecord	hitRecord;
	if (!this->hit(ray, hitRecord, T_MIN, T_MAX))
	{
		return (0.0);
	}

	double area = this->_height * this->_width;
	double distanceSquared = hitRecord.t0 * hitRecord.t0 * Utilities::vectorLengthSquared(vec);
	double cosine = fabs(Utilities::dot(vec, hitRecord.normal) / Utilities::vectorLength(vec));

	return (distanceSquared / (cosine * area));
}

// Returns a random point inside the rectangle's area
Vector3 Rectangle::random(const Vector3& origin) const
{
	Vector3 normal;
	Vector3 widthAxis;
	Vector3 heightAxis;

	if (!buildRectangleBasis(this->_transform.getOrientation(), normal, widthAxis, heightAxis))
	{
		return (Vector3(0.0, 0.0, 0.0));
	}

	const Sampler::Sample2D sample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	const Vector3 randomPointInsideRectangle = this->_transform.getPosition()
		+ (widthAxis * ((sample.x - 0.5) * this->_width))
		+ (heightAxis * ((sample.y - 0.5) * this->_height));

	return (Utilities::normalize(randomPointInsideRectangle - origin));
}

bool	Rectangle::sampleLight(const Vector3& origin, HittableLightSample& sample) const
{
	Vector3 normal;
	Vector3 widthAxis;
	Vector3 heightAxis;

	sample = HittableLightSample();
	if (!buildRectangleBasis(this->_transform.getOrientation(), normal, widthAxis, heightAxis))
	{
		return (false);
	}

	const Sampler::Sample2D surfaceSample = Sampler::sample2D(Sampler::DIM_LIGHT_SURFACE_POINT);
	const Vector3 randomPointInsideRectangle = this->_transform.getPosition()
		+ (widthAxis * ((surfaceSample.x - 0.5) * this->_width))
		+ (heightAxis * ((surfaceSample.y - 0.5) * this->_height));
	const Vector3 direction = randomPointInsideRectangle - origin;
	const double distanceSquared = Utilities::vectorLengthSquared(direction);
	if (!std::isfinite(distanceSquared) || distanceSquared <= 0.0)
	{
		return (false);
	}

	const double distance = std::sqrt(distanceSquared);
	sample.direction = direction / distance;

	const double area = this->_height * this->_width;
	const double cosine = std::fabs(Utilities::dot(sample.direction, normal));
	if (area <= 0.0 || cosine <= 0.0)
	{
		return (false);
	}

	sample.pdf = distanceSquared / (cosine * area);
	sample.tMax = distance;
	sample.material = this->_material.get();
	sample.valid = std::isfinite(sample.pdf) && sample.pdf > 0.0;
	return (sample.valid);
}

double	Rectangle::lightSelectionWeight(void) const
{
	if (!this->_material)
	{
		return (0.0);
	}

	const double area = this->_width * this->_height;
	const double luminance = Utilities::luminance(this->_material->emitted());
	if (area <= 0.0 || !std::isfinite(luminance) || luminance <= 0.0)
	{
		return (0.0);
	}
	return (area * luminance);
}
