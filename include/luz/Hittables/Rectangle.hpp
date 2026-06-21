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
		virtual Material* getMaterial(void) const override;
		void			setMaterial(std::shared_ptr<Material> material);
		void			setWidth(double width);
		void			setHeight(double height);
		void			setAxes(Vector3 widthAxis, Vector3 heightAxis);
		void			setTextureCoordinates(Vector3 uv0, Vector3 uv1, Vector3 uv2, Vector3 uv3);
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	hitAny(Ray& ray, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double  pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3 random(const Vector3& origin) const override;
		virtual bool	sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual bool	sampleEmission(HittableEmissionSample& sample) const override;
		virtual double	lightSelectionWeight(void) const override;

	private:
		bool	_buildBasis(Vector3& normal, Vector3& widthAxis, Vector3& heightAxis) const;
		Vector3	_uvAt(double u, double v) const;

		Transform   _transform;
		std::shared_ptr<Material>	_material;
		double	  _width;
		double	  _height;
		Vector3	 _widthAxis;
		Vector3	 _heightAxis;
		Vector3	 _uv0;
		Vector3	 _uv1;
		Vector3	 _uv2;
		Vector3	 _uv3;
		bool	 _hasAxes;
		bool	 _hasTextureCoordinates;
};
