#pragma once

#include "Materials/Material.hpp"
#include "AABB.hpp"
#include "Hittables/Hittable.hpp"
#include "IESProfile.hpp"
#include "Vector3.hpp"
#include <memory>

enum class SphereUVProjection
{
	LatLong,
	CubeCross
};

class	Sphere : public Hittable
{
	public:
		Sphere(void);
		Sphere(Vector3 position, double radius, std::shared_ptr<Material> material, SphereUVProjection uvProjection = SphereUVProjection::LatLong);
		Sphere(Vector3 position, double radius, std::shared_ptr<Material> material, bool visible, SphereUVProjection uvProjection = SphereUVProjection::LatLong);
		Vector3				getPosition(void) const;
		void				setPosition(Vector3 position);
		double				getRadius(void) const;
		void				setRadius(double radius);
		bool				isVisible(void) const;
		void				setVisible(bool visible);
		void				setIESProfile(std::shared_ptr<IESProfile> iesProfile, Vector3 direction, double rotationDegrees);
		SphereUVProjection	getUVProjection(void) const;
		void				setUVProjection(SphereUVProjection uvProjection);
		virtual Material*	getMaterial(void) const override;
		void				setMaterial(std::shared_ptr<Material> material);
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		hitAny(Ray& ray, double t_min, double t_max) const override;
		virtual bool		hitInterval(Ray& ray, double t_min, double t_max, double& t0, double& t1) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double  	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3 	random(const Vector3& origin) const override;
		virtual bool		sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual bool		sampleEmission(HittableEmissionSample& sample) const override;
		virtual double		lightSelectionWeight(void) const override;
		Vector3				randomToSphere(double distanceSquared) const;

	private:
		Vector3		_position;
		std::shared_ptr<Material>	_material;
		double		_radius;
		bool		_visible;
		SphereUVProjection	_uvProjection;
		std::shared_ptr<IESProfile>	_iesProfile;
		Vector3		_iesDirection;
		double		_iesRotationDegrees;
};
