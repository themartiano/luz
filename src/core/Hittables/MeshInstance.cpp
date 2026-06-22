#include "Hittables/MeshInstance.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Sampler.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
	constexpr double	INSTANCE_DEGREES_TO_RADIANS = 3.14159265358979323846 / 180.0;
	constexpr double	INSTANCE_SCALE_EPSILON = 1e-12;
	constexpr double	INSTANCE_NORMAL_EPSILON_SQUARED = 1e-24;

	bool	isFiniteVector(const Vector3& vector)
	{
		return (
			std::isfinite(vector.getX())
			&& std::isfinite(vector.getY())
			&& std::isfinite(vector.getZ())
		);
	}

	Vector3	normalizedOrZero(const Vector3& vector)
	{
		const double lengthSquared = Utilities::vectorLengthSquared(vector);

		if (!std::isfinite(lengthSquared) || lengthSquared <= INSTANCE_NORMAL_EPSILON_SQUARED)
		{
			return (Vector3());
		}
		return (vector / std::sqrt(lengthSquared));
	}

	double	sanitizedArea(double area)
	{
		if (!std::isfinite(area) || area <= 0.0)
		{
			return (0.0);
		}
		return (area);
	}
}

MeshInstance::InstanceTransform::InstanceTransform(Vector3 position, Vector3 rotationDegrees, Vector3 scale)
	: position(position), scale(scale)
{
	const double xRadians = rotationDegrees.getX() * INSTANCE_DEGREES_TO_RADIANS;
	const double yRadians = rotationDegrees.getY() * INSTANCE_DEGREES_TO_RADIANS;
	const double zRadians = rotationDegrees.getZ() * INSTANCE_DEGREES_TO_RADIANS;
	const double absX = std::fabs(scale.getX());
	const double absY = std::fabs(scale.getY());
	const double absZ = std::fabs(scale.getZ());

	this->cosX = std::cos(xRadians);
	this->sinX = std::sin(xRadians);
	this->cosY = std::cos(yRadians);
	this->sinY = std::sin(yRadians);
	this->cosZ = std::cos(zRadians);
	this->sinZ = std::sin(zRadians);
	this->invertible =
		absX > INSTANCE_SCALE_EPSILON
		&& absY > INSTANCE_SCALE_EPSILON
		&& absZ > INSTANCE_SCALE_EPSILON;
	this->uniformArea =
		std::fabs(absX - absY) <= INSTANCE_SCALE_EPSILON
		&& std::fabs(absX - absZ) <= INSTANCE_SCALE_EPSILON;
	this->uniformAreaScale = this->uniformArea ? absX * absX : 1.0;
}

Vector3	MeshInstance::InstanceTransform::rotate(Vector3 vector) const
{
	double x = vector.getX();
	double y = vector.getY();
	double z = vector.getZ();

	double nextY = (y * this->cosX) - (z * this->sinX);
	double nextZ = (y * this->sinX) + (z * this->cosX);
	y = nextY;
	z = nextZ;

	double nextX = (x * this->cosY) + (z * this->sinY);
	nextZ = (-x * this->sinY) + (z * this->cosY);
	x = nextX;
	z = nextZ;

	nextX = (x * this->cosZ) - (y * this->sinZ);
	nextY = (x * this->sinZ) + (y * this->cosZ);

	return (Vector3(nextX, nextY, z));
}

Vector3	MeshInstance::InstanceTransform::inverseRotate(Vector3 vector) const
{
	double x = vector.getX();
	double y = vector.getY();
	double z = vector.getZ();

	double nextX = (x * this->cosZ) + (y * this->sinZ);
	double nextY = (-x * this->sinZ) + (y * this->cosZ);
	x = nextX;
	y = nextY;

	nextX = (x * this->cosY) - (z * this->sinY);
	double nextZ = (x * this->sinY) + (z * this->cosY);
	x = nextX;
	z = nextZ;

	nextY = (y * this->cosX) + (z * this->sinX);
	nextZ = (-y * this->sinX) + (z * this->cosX);

	return (Vector3(x, nextY, nextZ));
}

Vector3	MeshInstance::InstanceTransform::transformPoint(const Vector3& point) const
{
	return (this->rotate(point * this->scale) + this->position);
}

Vector3	MeshInstance::InstanceTransform::inverseTransformPoint(const Vector3& point) const
{
	if (!this->invertible)
	{
		return (Vector3());
	}

	const Vector3 unrotated = this->inverseRotate(point - this->position);
	return (Vector3(
		unrotated.getX() / this->scale.getX(),
		unrotated.getY() / this->scale.getY(),
		unrotated.getZ() / this->scale.getZ()
	));
}

Vector3	MeshInstance::InstanceTransform::inverseTransformVector(const Vector3& vector) const
{
	if (!this->invertible)
	{
		return (Vector3());
	}

	const Vector3 unrotated = this->inverseRotate(vector);
	return (Vector3(
		unrotated.getX() / this->scale.getX(),
		unrotated.getY() / this->scale.getY(),
		unrotated.getZ() / this->scale.getZ()
	));
}

Vector3	MeshInstance::InstanceTransform::transformNormal(const Vector3& normal) const
{
	if (!this->invertible)
	{
		return (Vector3());
	}

	const Vector3 inverseScaled(
		normal.getX() / this->scale.getX(),
		normal.getY() / this->scale.getY(),
		normal.getZ() / this->scale.getZ()
	);

	return (normalizedOrZero(this->rotate(inverseScaled)));
}

MeshInstance::MeshInstance(
	std::shared_ptr<const Mesh> geometry,
	Vector3 position,
	Vector3 rotationDegrees,
	Vector3 scale,
	std::shared_ptr<Material> material
)
	: _geometry(std::move(geometry)),
	  _material(std::move(material)),
	  _transform(position, rotationDegrees, scale)
{
	this->_hasBoundingBox = this->_computeBoundingBox();
}

const std::shared_ptr<const Mesh>&	MeshInstance::getGeometry(void) const
{
	return (this->_geometry);
}

Material*	MeshInstance::getMaterial(void) const
{
	return (this->_material.get());
}

bool	MeshInstance::_computeBoundingBox(void)
{
	if (!this->_geometry)
	{
		return (false);
	}

	AABB localBox;
	if (!this->_geometry->createBoundingBox(localBox))
	{
		return (false);
	}

	const Vector3& localMinimum = localBox.getMinimum();
	const Vector3& localMaximum = localBox.getMaximum();
	const std::array<Vector3, 8> corners = {
		Vector3(localMinimum.getX(), localMinimum.getY(), localMinimum.getZ()),
		Vector3(localMaximum.getX(), localMinimum.getY(), localMinimum.getZ()),
		Vector3(localMinimum.getX(), localMaximum.getY(), localMinimum.getZ()),
		Vector3(localMaximum.getX(), localMaximum.getY(), localMinimum.getZ()),
		Vector3(localMinimum.getX(), localMinimum.getY(), localMaximum.getZ()),
		Vector3(localMaximum.getX(), localMinimum.getY(), localMaximum.getZ()),
		Vector3(localMinimum.getX(), localMaximum.getY(), localMaximum.getZ()),
		Vector3(localMaximum.getX(), localMaximum.getY(), localMaximum.getZ())
	};
	double minX = std::numeric_limits<double>::infinity();
	double minY = std::numeric_limits<double>::infinity();
	double minZ = std::numeric_limits<double>::infinity();
	double maxX = -std::numeric_limits<double>::infinity();
	double maxY = -std::numeric_limits<double>::infinity();
	double maxZ = -std::numeric_limits<double>::infinity();
	bool hasCorner = false;

	for (const Vector3& corner : corners)
	{
		const Vector3 worldCorner = this->_transform.transformPoint(corner);
		if (!isFiniteVector(worldCorner))
		{
			continue;
		}

		minX = std::min(minX, worldCorner.getX());
		minY = std::min(minY, worldCorner.getY());
		minZ = std::min(minZ, worldCorner.getZ());
		maxX = std::max(maxX, worldCorner.getX());
		maxY = std::max(maxY, worldCorner.getY());
		maxZ = std::max(maxZ, worldCorner.getZ());
		hasCorner = true;
	}
	if (!hasCorner)
	{
		return (false);
	}

	this->_boundingBox = AABB(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ));
	return (true);
}

bool	MeshInstance::_localRay(const Ray& ray, Ray& localRay, double& localDistanceScale) const
{
	if (!this->_geometry || !this->_transform.invertible)
	{
		return (false);
	}

	const Vector3 localDirection = this->_transform.inverseTransformVector(ray.getDirection());
	localDistanceScale = Utilities::vectorLength(localDirection);
	if (!std::isfinite(localDistanceScale) || localDistanceScale <= 0.0)
	{
		return (false);
	}

	const Vector3 localOrigin = this->_transform.inverseTransformPoint(ray.getOrigin());
	if (!isFiniteVector(localOrigin))
	{
		return (false);
	}
	localRay = Ray::fromNormalizedDirection(localOrigin, localDirection / localDistanceScale);
	return (true);
}

void	MeshInstance::_transformHitRecord(
	const Ray& worldRay,
	const HitRecord& localHitRecord,
	double localDistanceScale,
	HitRecord& worldHitRecord
) const
{
	const double worldT = localHitRecord.t0 / localDistanceScale;
	Vector3 worldNormal = this->_transform.transformNormal(localHitRecord.normal);
	Vector3 worldGeometricNormal = this->_transform.transformNormal(localHitRecord.geometricNormal);

	if (Utilities::dot(worldRay.getDirection(), worldNormal) > 0.0)
	{
		worldNormal = worldNormal * -1.0;
	}
	if (Utilities::dot(worldRay.getDirection(), worldGeometricNormal) > 0.0)
	{
		worldGeometricNormal = worldGeometricNormal * -1.0;
	}

	worldHitRecord = localHitRecord;
	worldHitRecord.t0 = worldT;
	worldHitRecord.position = worldRay.pointAtRay(worldT);
	worldHitRecord.normal = worldNormal;
	worldHitRecord.geometricNormal = worldGeometricNormal;
	worldHitRecord.frontFace = Utilities::dot(worldRay.getDirection(), worldNormal) < 0.0;
	worldHitRecord.material = this->_material.get();
}

bool	MeshInstance::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	Ray localRay;
	double localDistanceScale = 1.0;

	if (!this->_localRay(ray, localRay, localDistanceScale))
	{
		return (false);
	}

	HitRecord localHitRecord;
	if (!this->_geometry->hit(localRay, localHitRecord, t_min * localDistanceScale, t_max * localDistanceScale))
	{
		return (false);
	}

	const double worldT = localHitRecord.t0 / localDistanceScale;
	if (!std::isfinite(worldT) || worldT < t_min || worldT > t_max)
	{
		return (false);
	}

	this->_transformHitRecord(ray, localHitRecord, localDistanceScale, hitRecord);
	return (true);
}

bool	MeshInstance::hitAny(Ray& ray, double t_min, double t_max) const
{
	Ray localRay;
	double localDistanceScale = 1.0;

	if (!this->_localRay(ray, localRay, localDistanceScale))
	{
		return (false);
	}
	return (this->_geometry->hitAny(localRay, t_min * localDistanceScale, t_max * localDistanceScale));
}

bool	MeshInstance::createBoundingBox(AABB& outputBoundingBox) const
{
	if (!this->_hasBoundingBox)
	{
		return (false);
	}

	outputBoundingBox = this->_boundingBox;
	return (true);
}

double	MeshInstance::_surfaceArea(void) const
{
	std::call_once(this->_surfaceAreaOnce, [this] {
		if (!this->_geometry)
		{
			this->_surfaceAreaCache = 0.0;
			return;
		}
		if (this->_transform.uniformArea)
		{
			this->_surfaceAreaCache = sanitizedArea(
				this->_geometry->surfaceArea() * this->_transform.uniformAreaScale
			);
			return;
		}

		this->_surfaceAreaCache = sanitizedArea(this->_geometry->transformedSurfaceArea(
			[this](const Vector3& point) {
				return (this->_transform.transformPoint(point));
			}
		));
	});

	return (this->_surfaceAreaCache);
}

const std::vector<double>&	MeshInstance::_transformedAreaPrefixSums(void) const
{
	std::call_once(this->_transformedAreaPrefixOnce, [this] {
		if (!this->_geometry)
		{
			return;
		}

		const std::size_t elementCount = this->_geometry->surfaceElementCount();
		double totalArea = 0.0;

		this->_transformedAreaPrefixCache.reserve(elementCount);
		for (std::size_t index = 0; index < elementCount; index++)
		{
			totalArea += this->_geometry->transformedSurfaceElementArea(
				index,
				[this](const Vector3& point) {
					return (this->_transform.transformPoint(point));
				}
			);
			this->_transformedAreaPrefixCache.push_back(totalArea);
		}
		if (!std::isfinite(totalArea) || totalArea <= 0.0)
		{
			this->_transformedAreaPrefixCache.clear();
		}
	});

	return (this->_transformedAreaPrefixCache);
}

bool	MeshInstance::_sampleSurface(Vector3& position, Vector3& normal, double& area) const
{
	if (!this->_geometry)
	{
		return (false);
	}

	Vector3 localPosition;
	Vector3 localNormal;

	if (this->_transform.uniformArea)
	{
		area = this->_surfaceArea();
		if (area <= 0.0)
		{
			return (false);
		}
		if (!this->_geometry->sampleSurface(localPosition, localNormal))
		{
			return (false);
		}
	}
	else
	{
		const std::vector<double>& prefixSums = this->_transformedAreaPrefixSums();
		if (prefixSums.empty())
		{
			return (false);
		}

		area = prefixSums.back();
		if (!std::isfinite(area) || area <= 0.0)
		{
			return (false);
		}

		const double targetArea = Sampler::sample1D(Sampler::DIM_LIGHT_SURFACE_SELECTION) * area;
		const auto areaIt = std::lower_bound(prefixSums.begin(), prefixSums.end(), targetArea);
		const std::size_t randomIndex = std::min<std::size_t>(
			static_cast<std::size_t>(areaIt - prefixSums.begin()),
			prefixSums.size() - 1
		);
		const double previousArea = randomIndex == 0 ? 0.0 : prefixSums[randomIndex - 1];
		const double selectedArea = prefixSums[randomIndex] - previousArea;
		if (selectedArea <= 0.0)
		{
			return (false);
		}
		if (!this->_geometry->sampleSurfaceElement(randomIndex, localPosition, localNormal))
		{
			return (false);
		}
	}

	position = this->_transform.transformPoint(localPosition);
	normal = this->_transform.transformNormal(localNormal);
	return (isFiniteVector(position) && Utilities::vectorLengthSquared(normal) > 0.0);
}

double	MeshInstance::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	const double area = this->_surfaceArea();
	if (area <= 0.0)
	{
		return (0.0);
	}

	const double vecLength = Utilities::vectorLength(vec);
	if (!std::isfinite(vecLength) || vecLength <= 0.0)
	{
		return (0.0);
	}

	Ray ray(origin, vec);
	HitRecord hitRecord;
	if (!this->hit(ray, hitRecord, T_MIN, T_MAX))
	{
		return (0.0);
	}

	const double distanceSquared = hitRecord.t0 * hitRecord.t0 * Utilities::vectorLengthSquared(vec);
	const double cosine = std::fabs(Utilities::dot(vec, hitRecord.geometricNormal) / vecLength);
	if (!std::isfinite(distanceSquared) || !std::isfinite(cosine) || cosine <= 0.0)
	{
		return (0.0);
	}

	return (distanceSquared / (cosine * area));
}

Vector3	MeshInstance::random(const Vector3& origin) const
{
	HittableLightSample sample;

	if (this->sampleLight(origin, sample) && sample.valid)
	{
		return (sample.direction);
	}
	return (Hittable::random(origin));
}

bool	MeshInstance::sampleLight(const Vector3& origin, HittableLightSample& sample) const
{
	sample = HittableLightSample();

	Vector3 position;
	Vector3 normal;
	double area = 0.0;
	if (!this->_sampleSurface(position, normal, area))
	{
		return (false);
	}

	const Vector3 direction = position - origin;
	const double distanceSquared = Utilities::vectorLengthSquared(direction);
	if (!std::isfinite(distanceSquared) || distanceSquared <= 0.0)
	{
		return (false);
	}

	const double distance = std::sqrt(distanceSquared);
	sample.direction = direction / distance;

	const double cosine = std::fabs(Utilities::dot(sample.direction, normal));
	if (!std::isfinite(cosine) || cosine <= 0.0)
	{
		return (false);
	}

	sample.pdf = distanceSquared / (cosine * area);
	sample.tMax = distance;
	sample.material = this->_material.get();
	sample.valid =
		std::isfinite(sample.pdf)
		&& sample.pdf > 0.0
		&& std::isfinite(sample.tMax)
		&& sample.tMax > T_MIN;
	return (sample.valid);
}

bool	MeshInstance::sampleEmission(HittableEmissionSample& sample) const
{
	sample = HittableEmissionSample();
	if (!this->_material)
	{
		return (false);
	}

	const Color emitted = this->_material->emitted();
	if (Utilities::luminance(emitted) <= 0.0)
	{
		return (false);
	}

	Vector3 position;
	Vector3 normal;
	double area = 0.0;
	if (!this->_sampleSurface(position, normal, area))
	{
		return (false);
	}

	if (Sampler::sample1D(Sampler::DIM_LIGHT_EMISSION_SIDE) < 0.5)
	{
		normal = normal * -1.0;
	}
	const ONB basis(normal);

	sample.position = position;
	sample.normal = normal;
	sample.direction = normalizedOrZero(basis.local(Sampler::cosineHemisphere(Sampler::DIM_LIGHT_EMISSION_DIRECTION)));
	sample.emitted = emitted;
	sample.powerScale = 2.0 * D_PI * area;
	sample.valid =
		std::isfinite(sample.powerScale)
		&& sample.powerScale > 0.0
		&& Utilities::vectorLengthSquared(sample.direction) > 0.0;
	return (sample.valid);
}

double	MeshInstance::lightSelectionWeight(void) const
{
	if (!this->_material)
	{
		return (0.0);
	}

	const double luminance = Utilities::luminance(this->_material->emitted());
	const double area = this->_surfaceArea();
	if (!std::isfinite(luminance) || luminance <= 0.0 || area <= 0.0)
	{
		return (0.0);
	}
	return (area * luminance);
}
