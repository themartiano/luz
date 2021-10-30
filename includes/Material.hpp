#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "Color.hpp"

class	Material
{
	public:
		Material(void);
		Material(Color color, float opacity, float metallic, float albedo, float reflectionFuzziness, bool isDielectric, bool isEmissive, float lightIntensity);
		Color	getColor(void) const;
		void	setColor(Color color);
		float	getMetallic(void) const;
		float	getAlbedo(void) const;
		float	getReflectionFuzziness(void) const;
		bool	getIsDielectric(void) const;
		bool	getIsEmissive(void) const;
		float	getLightIntensity(void) const;

	private:
		Color	_color;
		float	_metallic;
		float	_albedo;
		float	_reflectionFuzziness;
		bool	_isDielectric;
		bool	_isEmissive;
		float	_lightIntensity;
		float	_opacity; //work on it
};

#endif