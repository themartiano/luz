#pragma once

#include "Color.hpp"
#include "Ray/Ray.hpp"
#include "MaterialTypes.hpp"

class   Ray;

class	Material
{
	public:
		Material(void);
		Material(Color color);
		virtual ~Material(void) = default;
		Color	getColor(void) const;
		void	setColor(Color color);
		virtual bool	scatter(Ray& ray);
		virtual Color	emitted(void);
		virtual double	scatteringPDF(Ray& ray);
		virtual MaterialType	getType(void) const;

	protected:
		Color	_color;
};
