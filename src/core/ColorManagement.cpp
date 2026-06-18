#include "ColorManagement.hpp"
#include <algorithm>
#include <cmath>

namespace
{
	struct Matrix3
	{
		double	m[3][3];
	};

	double	clampUnit(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		if (value >= 1.0)
		{
			return (1.0);
		}
		return (value);
	}

	double	clampNonNegative(double value)
	{
		if (!std::isfinite(value) || value <= 0.0)
		{
			return (0.0);
		}
		return (value);
	}

	Color	multiply(const Matrix3& matrix, Color color)
	{
		return (Color(
			matrix.m[0][0] * color.getRed() + matrix.m[0][1] * color.getGreen() + matrix.m[0][2] * color.getBlue(),
			matrix.m[1][0] * color.getRed() + matrix.m[1][1] * color.getGreen() + matrix.m[1][2] * color.getBlue(),
			matrix.m[2][0] * color.getRed() + matrix.m[2][1] * color.getGreen() + matrix.m[2][2] * color.getBlue()
		));
	}

	Color	clampColor(Color color)
	{
		return (Color(
			clampUnit(color.getRed()),
			clampUnit(color.getGreen()),
			clampUnit(color.getBlue())
		));
	}

	Color	rrtAndOdtFit(Color color)
	{
		auto fitChannel = [](double value) -> double
		{
			if (!std::isfinite(value) || value <= 0.0)
			{
				return (0.0);
			}
			const double a = value * (value + 0.0245786) - 0.000090537;
			const double b = value * (0.983729 * value + 0.4329510) + 0.238081;
			if (b <= 0.0 || !std::isfinite(b))
			{
				return (0.0);
			}
			return (a / b);
		};

		return (Color(
			fitChannel(color.getRed()),
			fitChannel(color.getGreen()),
			fitChannel(color.getBlue())
		));
	}

	const Matrix3 LINEAR_SRGB_TO_ACESCG = {{
		{0.6131324224, 0.3395380158, 0.0474166960},
		{0.0701243808, 0.9163940113, 0.0134515239},
		{0.0205876575, 0.1095745716, 0.8697854040}
	}};

	const Matrix3 ACESCG_TO_LINEAR_SRGB = {{
		{1.7048586763, -0.6217160219, -0.0832993717},
		{-0.1300768242, 1.1407357748, -0.0105598017},
		{-0.0239640729, -0.1289755083, 1.1530140189}
	}};

	const Matrix3 ACESCG_TO_XYZ = {{
		{0.6624541811, 0.1340042065, 0.1561876870},
		{0.2722287168, 0.6740817658, 0.0536895174},
		{-0.0055746495, 0.0040607335, 1.0103391003}
	}};

	const Matrix3 XYZ_TO_ACESCG = {{
		{1.6410233797, -0.3248032942, -0.2364246952},
		{-0.6636628587, 1.6153315917, 0.0167563477},
		{0.0117218943, -0.0082844420, 0.9883948585}
	}};

	const Matrix3 ACES_INPUT_FIT = {{
		{0.59719, 0.35458, 0.04823},
		{0.07600, 0.90834, 0.01566},
		{0.02840, 0.13383, 0.83777}
	}};

	const Matrix3 ACES_OUTPUT_FIT = {{
		{1.60475, -0.53108, -0.07367},
		{-0.10208, 1.10813, -0.00605},
		{-0.00327, -0.07276, 1.07602}
	}};
}

const char*	ColorManagement::workingSpaceName(void)
{
	return ("ACEScg scene-linear RGB (AP1 primaries, ACES D60 white)");
}

double	ColorManagement::srgbToLinear(double value)
{
	if (!std::isfinite(value) || value <= 0.0)
	{
		return (0.0);
	}
	if (value >= 1.0)
	{
		return (1.0);
	}
	if (value <= 0.04045)
	{
		return (value / 12.92);
	}
	return (std::pow((value + 0.055) / 1.055, 2.4));
}

double	ColorManagement::linearToSRGB(double value)
{
	value = clampUnit(value);
	if (value <= 0.0031308)
	{
		return (12.92 * value);
	}
	return ((1.055 * std::pow(value, 1.0 / 2.4)) - 0.055);
}

Color	ColorManagement::decodeSRGB(Color color)
{
	return (Color(
		srgbToLinear(color.getRed()),
		srgbToLinear(color.getGreen()),
		srgbToLinear(color.getBlue())
	));
}

Color	ColorManagement::encodeSRGB(Color color)
{
	return (Color(
		linearToSRGB(color.getRed()),
		linearToSRGB(color.getGreen()),
		linearToSRGB(color.getBlue())
	));
}

Color	ColorManagement::acescgFromLinearSRGB(Color color)
{
	return (multiply(LINEAR_SRGB_TO_ACESCG, sanitizeSceneLinear(color)));
}

Color	ColorManagement::linearSRGBFromACEScg(Color color)
{
	return (multiply(ACESCG_TO_LINEAR_SRGB, color));
}

Color	ColorManagement::acescgFromSRGB(Color color)
{
	return (acescgFromLinearSRGB(decodeSRGB(color)));
}

Color	ColorManagement::encodedSRGBFromACEScg(Color color)
{
	return (encodeSRGB(sanitizeSceneLinear(linearSRGBFromACEScg(color))));
}

Color	ColorManagement::acescgFromXYZ(Color xyz)
{
	return (multiply(XYZ_TO_ACESCG, xyz));
}

Color	ColorManagement::xyzFromACEScg(Color color)
{
	return (multiply(ACESCG_TO_XYZ, color));
}

Color	ColorManagement::displayTransformToLinearSRGB(Color color)
{
	Color linearSRGB = sanitizeSceneLinear(linearSRGBFromACEScg(color));
	Color fitted = rrtAndOdtFit(multiply(ACES_INPUT_FIT, linearSRGB));

	return (clampColor(multiply(ACES_OUTPUT_FIT, fitted)));
}

Color	ColorManagement::sanitizeSceneLinear(Color color)
{
	return (Color(
		clampNonNegative(color.getRed()),
		clampNonNegative(color.getGreen()),
		clampNonNegative(color.getBlue())
	));
}

double	ColorManagement::luminance(Color color)
{
	return (
		color.getRed() * 0.2722287168
		+ color.getGreen() * 0.6740817658
		+ color.getBlue() * 0.0536895174
	);
}

const char*	ColorManagement::imageEncodingName(ImageColorEncoding encoding)
{
	switch (encoding)
	{
		case ImageColorEncoding::SceneLinearACEScg:
			return ("scene-linear ACEScg");
		case ImageColorEncoding::DisplayLinearSRGB:
			return ("display-linear sRGB");
		case ImageColorEncoding::DisplayEncodedSRGB:
			return ("display-encoded sRGB");
	}
	return ("unknown");
}

std::string	ColorManagement::imageEncodingDescription(ImageColorEncoding encoding)
{
	switch (encoding)
	{
		case ImageColorEncoding::SceneLinearACEScg:
			return ("Luz scene-linear ACEScg RGB, AP1 primaries, ACES D60 white");
		case ImageColorEncoding::DisplayLinearSRGB:
			return ("Luz display-linear sRGB RGB, D65 white");
		case ImageColorEncoding::DisplayEncodedSRGB:
			return ("Luz display-referred sRGB RGB, IEC 61966-2-1 transfer");
	}
	return ("Luz RGB image with unknown color encoding");
}
