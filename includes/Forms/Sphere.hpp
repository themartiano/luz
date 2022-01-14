#pragma once

#include "Material.hpp"
#include "AABB.hpp"
#include "Hittable.hpp"
#include "Vector3.hpp"

class	Sphere : public Hittable
{
	public:
		Sphere(void);
		Sphere(Vector3 position, double radius, Material material);
		Vector3				getPosition(void) const;
		void				setPosition(Vector3 position);
		double				getRadius(void) const;
		void				setRadius(double radius);
		virtual Material	getMaterial(void) const override;
		void				setMaterial(Material material);
		virtual bool		hit(Ray& ray, double t_max) const override;
		virtual bool		createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double  	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3 	random(const Vector3& origin) const override;
		Vector3				randomToSphere(double distanceSquared) const;

	private:
		Vector3		_position;
		Material	_material;
		double		_radius;
};
