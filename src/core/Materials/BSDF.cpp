#include "Materials/BSDF.hpp"
#include "Defaults.hpp"
#include "ONB.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cmath>

namespace
{
	constexpr double	MIN_ALPHA = 1e-4;
	constexpr double	MIN_DENOMINATOR = 1e-10;

	double	clampFinite(double value, double minimum, double maximum)
	{
		if (!std::isfinite(value))
		{
			return (minimum);
		}
		return (std::max(minimum, std::min(maximum, value)));
	}

	Color	colorFromScale(Color color, double scale)
	{
		if (!std::isfinite(scale) || scale <= 0.0)
		{
			return (Color(0.0, 0.0, 0.0));
		}
		return (color * scale);
	}

	Vector3	negate(Vector3 vector)
	{
		return (vector * -1.0);
	}

	Vector3	orientedHalfVector(
		const Vector3& normal,
		const Vector3& halfVector
	)
	{
		if (Utilities::dot(normal, halfVector) < 0.0)
		{
			return (halfVector * -1.0);
		}
		return (halfVector);
	}
}

double	BSDF::clamp01(double value)
{
	return (clampFinite(value, 0.0, 1.0));
}

double	BSDF::roughnessToAlpha(double roughness)
{
	roughness = clamp01(roughness);
	return (std::max(MIN_ALPHA, roughness * roughness));
}

double	BSDF::maxChannel(const Color& color)
{
	return (std::max(color.getRed(), std::max(color.getGreen(), color.getBlue())));
}

Color	BSDF::clampColor01(Color color)
{
	return (Color(
		clamp01(color.getRed()),
		clamp01(color.getGreen()),
		clamp01(color.getBlue())
	));
}

Color	BSDF::mixColor(Color a, Color b, double factor)
{
	factor = clamp01(factor);
	return (a * (1.0 - factor) + b * factor);
}

Color	BSDF::schlickFresnel(Color f0, double cosTheta)
{
	cosTheta = clamp01(cosTheta);
	const double factor = std::pow(1.0 - cosTheta, 5.0);

	f0 = clampColor01(f0);
	return (Color(
		f0.getRed() + (1.0 - f0.getRed()) * factor,
		f0.getGreen() + (1.0 - f0.getGreen()) * factor,
		f0.getBlue() + (1.0 - f0.getBlue()) * factor
	));
}

double	BSDF::dielectricFresnel(double cosTheta, double etaI, double etaT)
{
	cosTheta = clamp01(std::fabs(cosTheta));
	if (
		!std::isfinite(etaI)
		|| !std::isfinite(etaT)
		|| etaI <= 0.0
		|| etaT <= 0.0
	)
	{
		return (1.0);
	}

	const double sinThetaI = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
	const double sinThetaT = etaI / etaT * sinThetaI;
	if (sinThetaT >= 1.0)
	{
		return (1.0);
	}
	const double cosThetaT = std::sqrt(std::max(0.0, 1.0 - sinThetaT * sinThetaT));
	const double rParallel = (
		(etaT * cosTheta) - (etaI * cosThetaT)
	) / (
		(etaT * cosTheta) + (etaI * cosThetaT)
	);
	const double rPerpendicular = (
		(etaI * cosTheta) - (etaT * cosThetaT)
	) / (
		(etaI * cosTheta) + (etaT * cosThetaT)
	);

	return (clamp01(0.5 * (rParallel * rParallel + rPerpendicular * rPerpendicular)));
}

Color	BSDF::conductorFresnel(Color eta, Color extinctionCoefficient, double cosTheta)
{
	auto channel = [](double etaChannel, double kChannel, double cosValue)
	{
		if (
			!std::isfinite(etaChannel)
			|| !std::isfinite(kChannel)
			|| etaChannel <= 0.0
			|| kChannel < 0.0
		)
		{
			return (0.0);
		}

		const double cosThetaSquared = cosValue * cosValue;
		const double etaSquared = etaChannel * etaChannel;
		const double kSquared = kChannel * kChannel;
		const double etaK = etaSquared + kSquared;
		const double twoEtaCosTheta = 2.0 * etaChannel * cosValue;
		const double parallelDenominator = etaK * cosThetaSquared + twoEtaCosTheta + 1.0;
		const double perpendicularDenominator = etaK + twoEtaCosTheta + cosThetaSquared;
		if (
			std::fabs(parallelDenominator) <= MIN_DENOMINATOR
			|| std::fabs(perpendicularDenominator) <= MIN_DENOMINATOR
		)
		{
			return (0.0);
		}

		const double rParallel = (
			(etaK * cosThetaSquared - twoEtaCosTheta + 1.0)
			/ parallelDenominator
		);
		const double rPerpendicular = (
			(etaK - twoEtaCosTheta + cosThetaSquared)
			/ perpendicularDenominator
		);

		return (BSDF::clamp01(0.5 * (rParallel + rPerpendicular)));
	};

	cosTheta = clamp01(cosTheta);
	return (Color(
		channel(eta.getRed(), extinctionCoefficient.getRed(), cosTheta),
		channel(eta.getGreen(), extinctionCoefficient.getGreen(), cosTheta),
		channel(eta.getBlue(), extinctionCoefficient.getBlue(), cosTheta)
	));
}

Vector3	BSDF::safeNormalize(Vector3 vector, Vector3 fallback)
{
	const double lengthSquared = Utilities::vectorLengthSquared(vector);

	if (!std::isfinite(lengthSquared) || lengthSquared <= 0.0)
	{
		return (fallback);
	}
	return (vector / std::sqrt(lengthSquared));
}

bool	BSDF::sameHemisphere(const Vector3& normal, const Vector3& a, const Vector3& b)
{
	return (Utilities::dot(normal, a) * Utilities::dot(normal, b) > 0.0);
}

Vector3	BSDF::sampleGGXHalfVector(const Vector3& normal, double alpha, Sampler::Sample2D sample)
{
	alpha = std::max(MIN_ALPHA, alpha);
	const double alphaSquared = alpha * alpha;
	const double phi = 2.0 * D_PI * clamp01(sample.x);
	const double u = std::min(1.0 - 1e-7, clamp01(sample.y));
	const double cosTheta = std::sqrt((1.0 - u) / (1.0 + (alphaSquared - 1.0) * u));
	const double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
	const ONB basis(normal);

	return (basis.local(
		sinTheta * std::cos(phi),
		sinTheta * std::sin(phi),
		cosTheta
	));
}

double	BSDF::ggxD(double noH, double alpha)
{
	noH = clamp01(noH);
	alpha = std::max(MIN_ALPHA, alpha);
	const double alphaSquared = alpha * alpha;
	const double denominator = noH * noH * (alphaSquared - 1.0) + 1.0;
	const double value = alphaSquared / (D_PI * denominator * denominator);

	return (std::isfinite(value) ? value : 0.0);
}

double	BSDF::ggxSmithG1(double noW, double alpha)
{
	noW = clamp01(noW);
	alpha = std::max(MIN_ALPHA, alpha);
	if (noW <= 0.0)
	{
		return (0.0);
	}

	const double alphaSquared = alpha * alpha;
	const double denominator = noW + std::sqrt(alphaSquared + (1.0 - alphaSquared) * noW * noW);
	if (denominator <= MIN_DENOMINATOR || !std::isfinite(denominator))
	{
		return (0.0);
	}
	return (2.0 * noW / denominator);
}

double	BSDF::ggxSmithG(double noV, double noL, double alpha)
{
	return (ggxSmithG1(noV, alpha) * ggxSmithG1(noL, alpha));
}

double	BSDF::ggxReflectionPDF(
	const Vector3& normal,
	const Vector3& view,
	const Vector3& scatteredDirection,
	double alpha
)
{
	const double noV = Utilities::dot(normal, view);
	const double noL = Utilities::dot(normal, scatteredDirection);
	if (noV <= 0.0 || noL <= 0.0)
	{
		return (0.0);
	}

	const Vector3 halfVector = safeNormalize(view + scatteredDirection, normal);
	const double voH = Utilities::dot(view, halfVector);
	const double noH = Utilities::dot(normal, halfVector);
	if (voH <= MIN_DENOMINATOR || noH <= 0.0)
	{
		return (0.0);
	}

	const double pdf = ggxD(noH, alpha) * noH / (4.0 * voH);
	return ((std::isfinite(pdf) && pdf > 0.0) ? pdf : 0.0);
}

Color	BSDF::ggxReflectionBSDFCos(
	Color fresnel,
	const Vector3& normal,
	const Vector3& view,
	const Vector3& scatteredDirection,
	double alpha
)
{
	const double noV = Utilities::dot(normal, view);
	const double noL = Utilities::dot(normal, scatteredDirection);
	if (noV <= 0.0 || noL <= 0.0)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	const Vector3 halfVector = safeNormalize(view + scatteredDirection, normal);
	const double noH = Utilities::dot(normal, halfVector);
	const double scale = ggxD(noH, alpha) * ggxSmithG(noV, noL, alpha) / (4.0 * noV);

	return (colorFromScale(clampColor01(fresnel), scale));
}

Color	BSDF::ggxConductorBSDFCos(
	Color eta,
	Color extinctionCoefficient,
	const Vector3& normal,
	const Vector3& view,
	const Vector3& scatteredDirection,
	double alpha
)
{
	const Vector3 halfVector = safeNormalize(view + scatteredDirection, normal);
	const double voH = std::max(0.0, Utilities::dot(view, halfVector));

	return (ggxReflectionBSDFCos(
		conductorFresnel(eta, extinctionCoefficient, voH),
		normal,
		view,
		scatteredDirection,
		alpha
	));
}

Color	BSDF::ggxSchlickReflectionBSDFCos(
	Color f0,
	const Vector3& normal,
	const Vector3& view,
	const Vector3& scatteredDirection,
	double alpha
)
{
	const Vector3 halfVector = safeNormalize(view + scatteredDirection, normal);
	const double voH = std::max(0.0, Utilities::dot(view, halfVector));

	return (ggxReflectionBSDFCos(
		schlickFresnel(f0, voH),
		normal,
		view,
		scatteredDirection,
		alpha
	));
}

bool	BSDF::refractDirection(
	const Vector3& incident,
	const Vector3& microfacetNormal,
	double eta,
	Vector3& refracted
)
{
	const double cosTheta = std::min(1.0, Utilities::dot(negate(incident), microfacetNormal));
	const Vector3 perpendicular = eta * (incident + cosTheta * microfacetNormal);
	const double perpendicularLengthSquared = Utilities::vectorLengthSquared(perpendicular);
	if (perpendicularLengthSquared > 1.0)
	{
		return (false);
	}

	const Vector3 parallel = -std::sqrt(std::max(0.0, 1.0 - perpendicularLengthSquared)) * microfacetNormal;
	refracted = safeNormalize(perpendicular + parallel, Utilities::reflect(incident, microfacetNormal));
	return (Utilities::vectorLengthSquared(refracted) > 0.0);
}

double	BSDF::ggxDielectricPDF(
	const Vector3& normal,
	const Vector3& view,
	const Vector3& scatteredDirection,
	double alpha,
	double etaI,
	double etaT
)
{
	const double noV = Utilities::dot(normal, view);
	const double noL = Utilities::dot(normal, scatteredDirection);
	if (noV <= 0.0 || noL == 0.0 || etaI <= 0.0 || etaT <= 0.0)
	{
		return (0.0);
	}

	if (noL > 0.0)
	{
		const Vector3 halfVector = safeNormalize(view + scatteredDirection, normal);
		const double fresnel = dielectricFresnel(Utilities::dot(view, halfVector), etaI, etaT);
		return (fresnel * ggxReflectionPDF(normal, view, scatteredDirection, alpha));
	}

	const double eta = etaI / etaT;
	Vector3 halfVector = safeNormalize(view + scatteredDirection * eta, normal);
	halfVector = orientedHalfVector(normal, halfVector);
	const double noH = Utilities::dot(normal, halfVector);
	const double voH = Utilities::dot(view, halfVector);
	const double loH = Utilities::dot(scatteredDirection, halfVector);
	if (noH <= 0.0 || voH <= 0.0 || loH >= 0.0)
	{
		return (0.0);
	}

	const double denominator = voH + eta * loH;
	if (std::fabs(denominator) <= MIN_DENOMINATOR)
	{
		return (0.0);
	}

	const double fresnel = dielectricFresnel(voH, etaI, etaT);
	const double dHalfDScattered = std::fabs((eta * eta * loH) / (denominator * denominator));
	const double pdf = (1.0 - fresnel) * ggxD(noH, alpha) * noH * dHalfDScattered;
	return ((std::isfinite(pdf) && pdf > 0.0) ? pdf : 0.0);
}

Color	BSDF::ggxDielectricBSDFCos(
	Color color,
	const Vector3& normal,
	const Vector3& view,
	const Vector3& scatteredDirection,
	double alpha,
	double etaI,
	double etaT
)
{
	const double noV = Utilities::dot(normal, view);
	const double noL = Utilities::dot(normal, scatteredDirection);
	if (noV <= 0.0 || noL == 0.0 || etaI <= 0.0 || etaT <= 0.0)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	color = clampColor01(color);
	if (noL > 0.0)
	{
		const Vector3 halfVector = safeNormalize(view + scatteredDirection, normal);
		const double fresnel = dielectricFresnel(Utilities::dot(view, halfVector), etaI, etaT);
		return (ggxReflectionBSDFCos(
			Color(fresnel, fresnel, fresnel),
			normal,
			view,
			scatteredDirection,
			alpha
		));
	}

	const double eta = etaI / etaT;
	Vector3 halfVector = safeNormalize(view + scatteredDirection * eta, normal);
	halfVector = orientedHalfVector(normal, halfVector);
	const double noH = Utilities::dot(normal, halfVector);
	const double voH = Utilities::dot(view, halfVector);
	const double loH = Utilities::dot(scatteredDirection, halfVector);
	if (noH <= 0.0 || voH <= 0.0 || loH >= 0.0)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	const double denominator = voH + eta * loH;
	if (std::fabs(denominator) <= MIN_DENOMINATOR)
	{
		return (Color(0.0, 0.0, 0.0));
	}

	const double fresnel = dielectricFresnel(voH, etaI, etaT);
	const double scale = (
		(1.0 - fresnel)
		* ggxD(noH, alpha)
		* ggxSmithG(noV, std::fabs(noL), alpha)
		* eta
		* eta
		* std::fabs(voH * loH)
		/ (noV * denominator * denominator)
	);

	return (colorFromScale(color, scale));
}
