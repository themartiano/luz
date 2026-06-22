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
		const Vector3&	getVertex0(void) const;
		const Vector3&	getVertex1(void) const;
		const Vector3&	getVertex2(void) const;
		void			setVertexNormals(Vector3 normal0, Vector3 normal1, Vector3 normal2);
		void			setTextureCoordinates(Vector3 uv0, Vector3 uv1, Vector3 uv2);
		virtual Material* getMaterial(void) const override;
		void			setMaterial(std::shared_ptr<Material> material);
		virtual bool	hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const override;
		virtual bool	hitAny(Ray& ray, double t_min, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;
		virtual double	pdfValue(const Vector3& origin, const Vector3& vec) const override;
		virtual Vector3	random(const Vector3& origin) const override;
		virtual bool	sampleLight(const Vector3& origin, HittableLightSample& sample) const override;
		virtual bool	sampleEmission(HittableEmissionSample& sample) const override;
		virtual double	lightSelectionWeight(void) const override;
		double			area(void) const;
		bool			sampleSurface(Vector3& position, Vector3& normal) const;

	private:
		void	_updateCache(void);

		Vector3	 _vertex0;
		Vector3	 _vertex1;
		Vector3	 _vertex2;
		Vector3	 _normal0;
		Vector3	 _normal1;
		Vector3	 _normal2;
		Vector3	 _uv0;
		Vector3	 _uv1;
		Vector3	 _uv2;
		Vector3	 _edge1;
		Vector3	 _edge2;
		Vector3	 _faceNormal;
		AABB	 _boundingBox;
		double	 _area;
		bool	 _hasVertexNormals;
		bool	 _hasTextureCoordinates;
		bool	 _isDegenerate;
		std::shared_ptr<Material>	_material;
};
