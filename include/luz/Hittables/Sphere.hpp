#pragma once

#include "Materials/Material.hpp"
#include "AABB.hpp"
#include "Hittables/Hittable.hpp"
#include "Vector3.hpp"

class	Sphere : public Hittable
{
	public:
		Sphere(void);
		Sphere(Vector3 position, double radius, std::shared_ptr<Material> material);
		Vector3				getPosition(void) const;
		void				setPosition(Vector3 position);
		double				getRadius(void) const;
		void				setRadius(double radius);
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		void				setMaterial(std::shared_ptr<Material> material);
		virtual bool		hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool		hitAny(Ray& ray, double t_min, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double  	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3 	random(const Vector3& origin) const override;
		virtual bool		sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual double		lightSelectionWeight(void) const override;
		Vector3				randomToSphere(double distanceSquared) const;

	private:
		Vector3		_position;
		std::shared_ptr<Material>	_material;
		double		_radius;
};
