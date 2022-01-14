#pragma once

#include "Hittable.hpp"
#include "Rectangle.hpp"
#include "Transform.hpp"
#include "Materials/Material.hpp"
#include <vector>

class   Cube : public Hittable
{
	public:
		Cube(void);
		Cube(Transform transform, double width, double height, double depth, std::shared_ptr<Material> material);
		void	setTransform(Transform transform);
		virtual std::shared_ptr<Material> getMaterial(void) const override;
		void	setMaterial(std::shared_ptr<Material> material);
		void	setWidth(double width);
		void	setHeight(double height);
		void	setDepth(double depth);
		virtual bool	hit(Ray& ray, double t_max) const override;
		virtual bool	createBoundingBox(AABB& outputBoundingBox) const override;

	private:
		Transform   _transform;
		std::shared_ptr<Material>	_material;
		double	  _width;
		double	  _height;
		double	  _depth;
		std::vector<Rectangle>  _faces;
		void	_generateFaces(void);
};
