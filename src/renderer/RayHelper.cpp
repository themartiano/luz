#include "RendererInternal.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include "Utilities.hpp"
#include <cmath>

// Generates a ray for the pixels 'x', 'y'
Ray	Renderer::internal::_generateRay(Scene& scene, std::size_t x, std::size_t y)
{
	const double width = scene.getImage()->getWidth();
	const double height = scene.getImage()->getHeight();
	const Camera camera = scene.getActiveCamera();

	double xU = double(x + randomEngine.doubleFloat()) / (width - 1);
	double yV = double(y + randomEngine.doubleFloat()) / (height - 1);

	const Vector3	cameraPosition = camera.getPosition();
	const Vector3	cameraLookDirection = camera.getDirection();

	const double	viewportWidth = 2.0 * tan(((camera.getFOV() * D_PI) / 180.0) / 2.0);
	const double	viewportHeight = (height / width) * viewportWidth;

	const double	lensRadius = camera.getAperture() / 2.0;
	const double	focusDistance = camera.getFocusDistance();

	const Vector3	w = Utilities::normalize(cameraLookDirection);
	const Vector3	viewUp(0.0, 1.0, 0.0);
	const Vector3	u = Utilities::normalize(Utilities::cross(viewUp, w));
	const Vector3	v = Utilities::cross(w, u);

	const Vector3	horizontal = u * viewportWidth * focusDistance;
	const Vector3	vertical = v * viewportHeight * focusDistance;
	const Vector3	lowerLeftCorner = cameraPosition + (horizontal / 2.0) + (vertical / 2.0) + (w * focusDistance);

	Vector3	offset(0.0, 0.0, 0.0);
	if (lensRadius > 0.0)
	{
		Vector3	rd = randomEngine.pointInsideUnitDisk() * lensRadius;
		offset = u * rd.getX() + v * rd.getY();
	}

	return (
		Ray(cameraPosition + offset, lowerLeftCorner - (horizontal * xU) - (vertical * yV) - cameraPosition - offset)
	);
}
