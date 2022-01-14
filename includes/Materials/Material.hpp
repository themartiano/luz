#pragma once

#include "Color.hpp"
#include "Ray/Ray.hpp"
#include "MaterialTypes.hpp"
#include "PDFs/PDF.hpp"
#include "Hittable.hpp"
#include <memory>

struct	HitRecord;

struct	ScatterRecord
{
	Ray	specularRay;
	bool	isSpecular;
	Color	attenuation;
	std::shared_ptr<PDF>	pdfPtr;
};

class	Material
{
	public:
		Material(void);
		Material(Color color);
		virtual ~Material(void) = default;
		Color	getColor(void) const;
		void	setColor(Color color);
		virtual bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		virtual Color	emitted(void);
		virtual double	scatteringPDF(Ray& ray, HitRecord& hitRecord);
		virtual MaterialType	getType(void) const;

	protected:
		Color	_color;
};
