#include "RendererInternal.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include "Utilities.hpp"
#include <cmath>

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
	const double	focusDistance = camera.getFocusDistance();

	const Vector3	w = Utilities::normalize(cameraLookDirection);
	const Vector3	viewUp(0.0, 1.0, 0.0);
	renderCamera.u = Utilities::normalize(Utilities::cross(viewUp, w));
	renderCamera.v = Utilities::cross(w, renderCamera.u);
	renderCamera.lensRadius = camera.getAperture() / 2.0;
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
