#include "RendererInternal.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include "Utilities.hpp"
#include <cmath>

namespace
{
	Vector3	cameraUpOrFallback(const Vector3& cameraUpDirection, const Vector3& lookDirection)
	{
		Vector3 viewUp = cameraUpDirection;

		if (
			!std::isfinite(viewUp.getX())
			|| !std::isfinite(viewUp.getY())
			|| !std::isfinite(viewUp.getZ())
			|| Utilities::vectorLengthSquared(viewUp) <= 1e-12
		)
		{
			viewUp = Vector3(0.0, 1.0, 0.0);
		}
		viewUp = Utilities::normalize(viewUp);
		if (Utilities::vectorLengthSquared(Utilities::cross(viewUp, lookDirection)) <= 1e-12)
		{
			viewUp = Vector3(0.0, 0.0, 1.0);
		}
		if (Utilities::vectorLengthSquared(Utilities::cross(viewUp, lookDirection)) <= 1e-12)
		{
			viewUp = Vector3(1.0, 0.0, 0.0);
		}
		return (viewUp);
	}
}

Renderer::internal::RenderCamera	Renderer::internal::_prepareRenderCamera(Scene& scene)
{
	RenderCamera renderCamera;

	renderCamera.width = scene.getImage()->getWidth();
	renderCamera.height = scene.getImage()->getHeight();
	renderCamera.inverseWidthMinusOne = renderCamera.width > 1.0 ? 1.0 / (renderCamera.width - 1.0) : 0.0;
	renderCamera.inverseHeightMinusOne = renderCamera.height > 1.0 ? 1.0 / (renderCamera.height - 1.0) : 0.0;

	const Camera camera = scene.getActiveCamera();

	renderCamera.position = camera.getPosition();
	const Vector3	cameraLookDirection = camera.getDirection();

	const double	viewportWidth = 2.0 * tan(((camera.getFOV() * D_PI) / 180.0) / 2.0);
	const double	viewportHeight = (renderCamera.height / renderCamera.width) * viewportWidth;
	const double	rawFocusDistance = camera.getFocusDistance();
	const bool		hasValidFocusDistance = std::isfinite(rawFocusDistance) && rawFocusDistance > 0.0;
	const double	focusDistance = hasValidFocusDistance ? rawFocusDistance : 1.0;

	const Vector3	w = Utilities::normalize(cameraLookDirection);
	const Vector3	viewUp = cameraUpOrFallback(camera.getUpDirection(), w);
	renderCamera.u = Utilities::normalize(Utilities::cross(viewUp, w));
	renderCamera.v = Utilities::cross(w, renderCamera.u);
	renderCamera.lensRadius = hasValidFocusDistance ? camera.getAperture() / 2.0 : 0.0;
	renderCamera.horizontal = renderCamera.u * viewportWidth * focusDistance;
	renderCamera.vertical = renderCamera.v * viewportHeight * focusDistance;
	renderCamera.lowerLeftCorner = renderCamera.position
		+ (renderCamera.horizontal / 2.0)
		+ (renderCamera.vertical / 2.0)
		+ (w * focusDistance);

	return (renderCamera);
}

// Generates a ray for the pixels 'x', 'y'
Ray	Renderer::internal::_generateRay(const RenderCamera& renderCamera, std::size_t x, std::size_t y)
{
	double xU = double(x + randomEngine.doubleFloat()) * renderCamera.inverseWidthMinusOne;
	double yV = double(y + randomEngine.doubleFloat()) * renderCamera.inverseHeightMinusOne;

	Vector3	offset(0.0, 0.0, 0.0);
	if (renderCamera.lensRadius > 0.0)
	{
		Vector3	rd = randomEngine.pointInsideUnitDisk() * renderCamera.lensRadius;
		offset = renderCamera.u * rd.getX() + renderCamera.v * rd.getY();
	}

	return (
		Ray(renderCamera.position + offset,
			renderCamera.lowerLeftCorner
				- (renderCamera.horizontal * xU)
				- (renderCamera.vertical * yV)
				- renderCamera.position
				- offset)
	);
}
