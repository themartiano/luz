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

		MaterialType	type = BASIC;

	protected:
		Color	_color;
};
