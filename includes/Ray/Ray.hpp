#pragma once

#include "Vector3.hpp"
#include "Materials/Material.hpp"
#include "PDFs/PDF.hpp"
#include "Ray/HitRecord.hpp"
#include "Ray/ScatterRecord.hpp"
#include <memory>

class	Material;
struct	ScatterRecord;

class   Ray
{
	public:
		Ray(void);
		Ray(Vector3 origin, Vector3 direction);
		Vector3 pointAtRay(double t);
		Vector3 getOrigin(void) const;
		void	setOrigin(Vector3 origin);
		Vector3 getDirection(void) const;
		void	setDirection(Vector3 direction);
		struct HitRecord		hitRecord;
		struct ScatterRecord	scatterRecord;

	private:
		Vector3 _origin;
		Vector3 _direction;

};
