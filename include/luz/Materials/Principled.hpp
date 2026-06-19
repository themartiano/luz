#pragma once

#include "Materials/Material.hpp"

enum	SubsurfaceMethod
{
	SUBSURFACE_BURLEY,
	SUBSURFACE_THIN
};

class	Principled : public Material
{
	public:
		Principled(void);
		Principled(Color color, double metallic, double roughness);
		Principled(
			Color color,
			double metallic,
			double roughness,
			double transmission,
			double refractiveIndex,
			double clearcoat,
			double clearcoatRoughness,
			double sheen
		);
		double	getMetallic(void) const;
		double	getRoughness(void) const;
		double	getTransmission(void) const;
		double	getRefractiveIndex(void) const;
		double	getClearcoat(void) const;
		double	getClearcoatRoughness(void) const;
		double	getSheen(void) const;
		double	getSubsurface(void) const;
		Color	getSubsurfaceRadius(void) const;
		double	getSubsurfaceScale(void) const;
		Color	getSubsurfaceColor(void) const;
		SubsurfaceMethod	getSubsurfaceMethod(void) const;
		bool	usesThinSubsurface(void) const;
		Color	getAbsorptionCoefficient(void) const;
		void	setMetallic(double metallic);
		void	setRoughness(double roughness);
		void	setTransmission(double transmission);
		void	setRefractiveIndex(double refractiveIndex);
		void	setClearcoat(double clearcoat);
		void	setClearcoatRoughness(double clearcoatRoughness);
		void	setSheen(double sheen);
		void	setSubsurface(double subsurface);
		void	setSubsurfaceRadius(Color radius);
		void	setSubsurfaceScale(double scaleMeters);
		void	setSubsurfaceColor(Color color);
		void	setSubsurfaceMethod(SubsurfaceMethod method);
		void	setSkinSubsurfaceProfile(void);
		void	setAbsorptionCoefficient(Color absorptionCoefficient);
		void	setTransmittance(Color transmittance, double distanceMeters);
		bool	scatter(Ray& ray, HitRecord& hitRecord, ScatterRecord& scatterRecord);
		Color	evaluateBSDFCos(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		double	scatteringPDF(
			const Ray& ray,
			const HitRecord& hitRecord,
			const Vector3& scatteredDirection
		) const;
		MaterialType	getType(void) const;

	private:
		double	_metallic;
		double	_roughness;
		double	_transmission;
		double	_refractiveIndex;
		double	_clearcoat;
		double	_clearcoatRoughness;
		double	_sheen;
		double	_subsurface;
		Color	_subsurfaceRadius;
		double	_subsurfaceScale;
		Color	_subsurfaceColor;
		SubsurfaceMethod	_subsurfaceMethod;
		Color	_absorptionCoefficient;
};
