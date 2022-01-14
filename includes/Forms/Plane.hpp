#pragma once

#include "Hittable.hpp"
#include "Vector3.hpp"
#include "Materials/Material.hpp"

class   Plane : public Hittable
{
	public:
		Plane(void);
		Plane(double y, Vector3 orientation, Material material);
		void			setY(double y);
		void			setOrientation(Vector3 orientation);
		virtual Material getMaterial(void) const override;
		void			setMaterial(Material material);
		virtual bool	hit(Ray& ray, double t_max) const override;
		virtual bool	createBoundingBox(AABB& boundingBox) const override;

	private:
		double	   _y;
		Vector3	 _orientation;
		Material	_material;

};
