#pragma once

#include "Hittables/Hittable.hpp"
#include "Vector3.hpp"
#include "Ray/Ray.hpp"
#include "AABB.hpp"
#include "Materials/Material.hpp"

class   Triangle : public Hittable
{
	public:
		Triangle(void);
		Triangle(Vector3 vertex0, Vector3 vertex1, Vector3 vertex2, std::shared_ptr<Material> material);
		Triangle(
			Vector3 vertex0,
			Vector3 vertex1,
			Vector3 vertex2,
			Vector3 normal0,
			Vector3 normal1,
			Vector3 normal2,
			std::shared_ptr<Material> material
		);
		void			setVertex0(Vector3 vertex0);
		void			setVertex1(Vector3 vertex1);
		void			setVertex2(Vector3 vertex2);
		virtual std::shared_ptr<Material> getMaterial(void) const override;
		void			setMaterial(std::shared_ptr<Material> material);
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3	random(const Vector3& origin) const override;
		double			area(void) const;

	private:
		Vector3	 _vertex0;
		Vector3	 _vertex1;
		Vector3	 _vertex2;
		Vector3	 _normal0;
		Vector3	 _normal1;
		Vector3	 _normal2;
		bool	 _hasVertexNormals;
		std::shared_ptr<Material>	_material;
};
