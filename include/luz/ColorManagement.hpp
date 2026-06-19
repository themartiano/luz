#pragma once

#include "Color.hpp"
#include <string>

enum class ImageColorEncoding
{
	SceneLinearACEScg,
	DisplayLinearSRGB,
	DisplayEncodedSRGB
};

enum class TextureColorRole
{
	ColorSRGB,
	LinearSRGB,
	Data
};

enum class ViewTransform
{
	Standard,
	AgX,
	ACES,
	Raw
};

namespace ColorManagement
{
	const char*	workingSpaceName(void);
	const char*	viewTransformName(ViewTransform viewTransform);
	const char*	viewTransformDescription(ViewTransform viewTransform);
	double		srgbToLinear(double value);
	double		linearToSRGB(double value);
	Color		decodeSRGB(Color color);
	Color		encodeSRGB(Color color);
	Color		acescgFromLinearSRGB(Color color);
	Color		linearSRGBFromACEScg(Color color);
	Color		acescgFromSRGB(Color color);
	Color		encodedSRGBFromACEScg(Color color);
	Color		acescgFromXYZ(Color xyz);
	Color		xyzFromACEScg(Color color);
	Color		viewTransformToLinearSRGB(Color color, ViewTransform viewTransform);
	Color		sanitizeSceneLinear(Color color);
	double		luminance(Color color);
	const char*	imageEncodingName(ImageColorEncoding encoding);
	std::string	imageEncodingDescription(ImageColorEncoding encoding);
}
