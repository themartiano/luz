#pragma once

#include "Color.hpp"
#include "Ray/Ray.hpp"
#include "MaterialTypes.hpp"
#include "ONB.hpp"
#include "Hittables/Hittable.hpp"
#include <memory>

struct	HitRecord;

enum	ScatterPDFType
{
	SCATTER_PDF_NONE,
	SCATTER_PDF_COSINE,
	SCATTER_PDF_SPHERE
};

struct	ScatterRecord
{
	Ray	specularRay;
	bool	isSpecular = false;
	Color	attenuation;
	ScatterPDFType	pdfType = SCATTER_PDF_NONE;
	ONB	cosineBasis;
};

class	Material
{
	public:
		Material(void);
		Material(Color color);
		virtual ~Material(void) = default;
		virtual Color	getColor(void) const;
		virtual void	setColor(Color color);
		virtual bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		virtual Color	emitted(void);
		virtual double	scatteringPDF(Ray& ray, HitRecord& hitRecord);
		virtual MaterialType	getType(void) const;

	protected:
		Color	_color;
};
