#pragma once

#include "Color.hpp"

class	Material
{
	public:
		Material(void);
		Material(Color color, double opacity, double metallic, double albedo, double reflectionFuzziness, bool isDielectric, bool isEmissive, double lightIntensity);
		Color	getColor(void) const;
		void	setColor(Color color);
		double	getMetallic(void) const;
		double	getAlbedo(void) const;
		double	getReflectionFuzziness(void) const;
		bool	getIsDielectric(void) const;
		bool	getIsEmissive(void) const;
		double	getLightIntensity(void) const;

	private:
		Color	_color;
		double	_metallic;
		double	_albedo;
		double	_reflectionFuzziness;
		bool	_isDielectric;
		bool	_isEmissive;
		double	_lightIntensity;
		double	_opacity; //work on it
};
