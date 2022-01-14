#pragma once

#include "Color.hpp"
#include "Ray/Ray.hpp"

class   Ray;

class	Material
{
	public:
		Material(void);
		Material(Color color);
		Color	getColor(void) const;
		void	setColor(Color color);
		virtual bool	scatter(Ray& ray);

	protected:
		Color	_color;
};
