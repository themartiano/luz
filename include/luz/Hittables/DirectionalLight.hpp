#pragma once

#include "Hittables/Hittable.hpp"
#include "Materials/Material.hpp"
#include "Vector3.hpp"
#include <memory>

class	DirectionalLight : public Hittable
{
	public:
		DirectionalLight(void);
		DirectionalLight(Vector3 direction, std::shared_ptr<Material> material);
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;
		virtual std::shared_ptr<Material>	getMaterial(void) const override;
		virtual double	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3	random(const Vector3& origin) const override;
		virtual bool	sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual double	lightSelectionWeight(void) const override;

	private:
		Vector3	_direction;
		std::shared_ptr<Material>	_material;
};
