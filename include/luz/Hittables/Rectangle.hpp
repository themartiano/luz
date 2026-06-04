#pragma once

#include "Hittables/Hittable.hpp"
#include "Vector3.hpp"
#include "Materials/Material.hpp"
#include "AABB.hpp"
#include "Transform.hpp"

class   Rectangle : public Hittable
{
	public:
		Rectangle(void);
		Rectangle(const Rectangle& toCopy);
		Rectangle(Transform transform, double width, double height, std::shared_ptr<Material> material);
		void			setTransform(Transform transform);
		virtual std::shared_ptr<Material> getMaterial(void) const override;
		void			setMaterial(std::shared_ptr<Material> material);
		void			setWidth(double width);
		void			setHeight(double height);
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double  pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3 random(const Vector3& origin) const override;

	private:
		Transform   _transform;
		std::shared_ptr<Material>	_material;
		double	  _width;
		double	  _height;
};
