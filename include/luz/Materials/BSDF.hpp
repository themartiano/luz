#pragma once

#include "Color.hpp"
#include "Sampler.hpp"
#include "Vector3.hpp"

namespace BSDF
{
	double	clamp01(double value);
	double	roughnessToAlpha(double roughness);
	double	maxChannel(const Color& color);
	Color	clampColor01(Color color);
	Color	mixColor(Color a, Color b, double factor);
	Color	schlickFresnel(Color f0, double cosTheta);
	double	dielectricFresnel(double cosTheta, double etaI, double etaT);
	Color	conductorFresnel(Color eta, Color extinctionCoefficient, double cosTheta);
	Vector3	safeNormalize(Vector3 vector, Vector3 fallback);
	bool	sameHemisphere(const Vector3& normal, const Vector3& a, const Vector3& b);

	Vector3	sampleGGXHalfVector(const Vector3& normal, double alpha, Sampler::Sample2D sample);
	double	ggxD(double noH, double alpha);
	double	ggxSmithG1(double noW, double alpha);
	double	ggxSmithG(double noV, double noL, double alpha);

	double	ggxReflectionPDF(
		const Vector3& normal,
		const Vector3& view,
		const Vector3& scatteredDirection,
		double alpha
	);
	Color	ggxReflectionBSDFCos(
		Color fresnel,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& scatteredDirection,
		double alpha
	);
	Color	ggxConductorBSDFCos(
		Color eta,
		Color extinctionCoefficient,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& scatteredDirection,
		double alpha
	);
	Color	ggxSchlickReflectionBSDFCos(
		Color f0,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& scatteredDirection,
		double alpha
	);

	bool	refractDirection(
		const Vector3& incident,
		const Vector3& microfacetNormal,
		double eta,
		Vector3& refracted
	);
	double	ggxDielectricPDF(
		const Vector3& normal,
		const Vector3& view,
		const Vector3& scatteredDirection,
		double alpha,
		double etaI,
		double etaT
	);
	Color	ggxDielectricBSDFCos(
		Color color,
		const Vector3& normal,
		const Vector3& view,
		const Vector3& scatteredDirection,
		double alpha,
		double etaI,
		double etaT
	);
}
