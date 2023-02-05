#include "Renderer/Renderer.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include "Utilities.hpp"
#include <cmath>

// Generates a ray for the pixels 'x', 'y'
Ray	Renderer::internal::_generateRay(Scene& scene, std::size_t x, std::size_t y)
{
	static double width = scene.getImage()->getWidth();
	static double height = scene.getImage()->getHeight();

	double xU = double(x + randomEngine.doubleFloat()) / (width - 1);
	double yV = double(y + randomEngine.doubleFloat()) / (height - 1);

	static Vector3	cameraPosition = scene.getActiveCamera().getPosition();
	static Vector3	cameraLookDirection = scene.getActiveCamera().getDirection();

	static double	viewportWidth = 2.0 * tan(((scene.getActiveCamera().getFOV() * D_PI) / 180.0) / 2.0);
	static double	viewportHeight = (height / width) * viewportWidth;

	static double	lensRadius = scene.getActiveCamera().getAperture() / 2.0;
	static double	focusDistance = scene.getActiveCamera().getFocusDistance();

	static Vector3	w = Utilities::normalize(cameraLookDirection);
	static Vector3	viewUp(0.0, 1.0, 0.0);
	static Vector3	u = Utilities::normalize(Utilities::cross(viewUp, w));
	static Vector3	v = Utilities::cross(w, u);

	static Vector3	horizontal = u * viewportWidth * focusDistance;
	static Vector3	vertical = v * viewportHeight * focusDistance;
	static Vector3	lowerLeftCorner = cameraPosition + (horizontal / 2.0) + (vertical / 2.0) + (w * focusDistance);

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
