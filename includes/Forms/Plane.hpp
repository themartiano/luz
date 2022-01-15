#pragma once

#include "Hittable.hpp"
#include "Vector3.hpp"
#include "Materials/Material.hpp"

class   Plane : public Hittable
{
	public:
		Plane(void);
		Plane(double y, Vector3 orientation, std::shared_ptr<Material> material);
		void			setY(double y);
		void			setOrientation(Vector3 orientation);
		virtual std::shared_ptr<Material> getMaterial(void) const override;
		void			setMaterial(std::shared_ptr<Material> material);
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& boundingBox) const override;

	private:
		double	   _y;
		Vector3	 _orientation;
		std::shared_ptr<Material>	_material;

};
