#include "Scene.hpp"

void	init_Luz(Scene &scene)
{
	scene.setXResolution(1920);
	scene.setYResolution(1080);

	scene.addCamera(Camera(Transform(Vector3(0.0f, 0.0f, 6.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)), 70));
	scene.addSphere(Sphere());
}
