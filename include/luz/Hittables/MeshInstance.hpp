#pragma once

#include "AABB.hpp"
#include "Hittables/Hittable.hpp"
#include "Hittables/Mesh.hpp"
#include "Materials/Material.hpp"
#include "Vector3.hpp"
#include <memory>
#include <mutex>
#include <vector>

class	MeshInstance : public Hittable
{
	public:
		MeshInstance(
			std::shared_ptr<const Mesh> geometry,
			Vector3 position,
			Vector3 rotationDegrees,
			Vector3 scale,
			std::shared_ptr<Material> material
		);

		const std::shared_ptr<const Mesh>&	getGeometry(void) const;
		virtual Material*	getMaterial(void) const override;
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		hitAny(Ray& ray, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3	random(const Vector3& origin) const override;
		virtual bool		sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual bool		sampleEmission(HittableEmissionSample& sample) const override;
		virtual double	lightSelectionWeight(void) const override;

	private:
		struct	InstanceTransform
		{
			Vector3	position;
			Vector3	scale;
			double	cosX = 1.0;
			double	sinX = 0.0;
			double	cosY = 1.0;
			double	sinY = 0.0;
			double	cosZ = 1.0;
			double	sinZ = 0.0;
			double	uniformAreaScale = 1.0;
			bool	invertible = true;
			bool	uniformArea = true;

			InstanceTransform(Vector3 position, Vector3 rotationDegrees, Vector3 scale);
			Vector3	rotate(Vector3 vector) const;
			Vector3	inverseRotate(Vector3 vector) const;
			Vector3	transformPoint(const Vector3& point) const;
			Vector3	inverseTransformPoint(const Vector3& point) const;
			Vector3	inverseTransformVector(const Vector3& vector) const;
			Vector3	transformNormal(const Vector3& normal) const;
		};

		double	_surfaceArea(void) const;
		const std::vector<double>&	_transformedAreaPrefixSums(void) const;
		bool	_sampleSurface(Vector3& position, Vector3& normal, double& area) const;
		bool	_computeBoundingBox(void);
		bool	_localRay(const Ray& ray, Ray& localRay, double& localDistanceScale) const;
		void	_transformHitRecord(
			const Ray& worldRay,
			const HitRecord& localHitRecord,
			double localDistanceScale,
			HitRecord& worldHitRecord
		) const;

		std::shared_ptr<const Mesh>	_geometry;
		std::shared_ptr<Material>	_material;
		InstanceTransform	_transform;
		AABB	_boundingBox;
		bool	_hasBoundingBox = false;
		mutable double	_surfaceAreaCache = 0.0;
		mutable std::once_flag	_surfaceAreaOnce;
		mutable std::vector<double>	_transformedAreaPrefixCache;
		mutable std::once_flag	_transformedAreaPrefixOnce;
};
