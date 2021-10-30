#include "Scene.hpp"
#include "Exit.hpp"
#include "BMP.hpp"
#include "Renderer.hpp"
#include "ANSIColors.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Rectangle.hpp"
#include "Material.hpp"
#include "BVHNode.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <iostream>

// Main function
int	main(int argc, char *argv[])
{
	Scene	scene;

	// if (argc <= 1)
	// 	exitError(scene, "Scene not specified.");

	std::cout << CLR_BLUE << "Preparing...\n\n" << CLR_RESET;
	//read scene file
	//check res

	if (argc >= 3)
	{
		//read_flags();
	}

	srand(time(0));

	scene.setXResolution(1920);
	scene.setYResolution(1080);
	scene.initializePixelArray();
	scene.setSampleCount(4);
	scene.setMaxLightBounces(8);
	scene.setGammaCorrected(true);
	scene.setRenderSky(false);

	// Current coordinate system ~~ Forward: -Z | Up: -Y | Right: -X
	scene.addCamera(Camera(Vector3(0.0f, 0.0f, 12.0f), Vector3(0.0f, 0.0f, -8.0f), 65, 0.1856321f));

	scene.addHittable(std::make_shared<Rectangle>(Transform(Vector3(-5.0f, -3.0f, -8.0f), Vector3(0.68f, -0.3f, 1.0f), Vector3(1.0f, 1.0f, 1.0f)), 5.0f, Material(Color(1.0f, 1.10f, 1.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, true, 10.0f)));

	// Glass
	scene.addHittable(std::make_shared<Sphere>(Vector3(-6.7f, 0.0f, -4.0f), Material(Color(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.5f, 0.0f, true, false, 0.0f), 3.0f));
	// Metal
	scene.addHittable(std::make_shared<Sphere>(Vector3(-3.35f, 0.0f, -8.0f), Material(Color(0.7f, 0.6f, 0.5f), 1.0f, 1.0f, 0.5f, 0.0f, false, false, 0.0f), 3.0f));

	// Emissive sphere
	//scene.addHittable(std::make_shared<Sphere>(Vector3(-3.35f, 0.0f, -8.0f), Material(Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.5f, 0.0f, false, true, 1.0f), 3.0f));

	// Lambertian
	scene.addHittable(std::make_shared<Sphere>(Vector3(0.0f, 0.0f, -12.0f), Material(Color(0.0f, 0.0f, 0.8f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f), 3.0f));

	// Ground (Lambertian)
	scene.addHittable(std::make_shared<Sphere>(Vector3(0.0f, 1003.0f, -8.0f), Material(Color(0.5f, 1.0f, 0.5f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f), 1000.0f));

	/*std::vector<std::shared_ptr<Hittable>> tinySpheres;
	for (int x = -11; x < 11; x++)
	{
		for (int y = -11; y < 11; y++)
		{
			if (x == 0)
				break;
			if (y == 0)
				continue;

			float	sphereRadius = randomFloat() / 2.0f;

			Vector3 spherePosition(float(x) * (3.6f * randomFloat()), 3.0f - sphereRadius, float(y) * (3.0f * randomFloat()));

			float	random = randomFloat();

			float	metallic = 0.0f;
			float	reflectionFuzziness = 0.0f;
			bool	glass = false;

			if (random >= 0.85f && random < 0.95f)
			{
				metallic = 1.0f;
				reflectionFuzziness = randomFloat() - 0.5f;
				reflectionFuzziness = reflectionFuzziness < 0.0f ? 0.0f : reflectionFuzziness;
			}
			else if (random >= 0.95f)
			{
				glass = true;
			}

			tinySpheres.push_back(std::make_shared<Sphere>(spherePosition, Material(Color(randomFloat(), randomFloat(), randomFloat()), 1.0f, metallic, 0.5f, reflectionFuzziness, glass), sphereRadius));
		}
	}
	scene.addHittable(std::make_shared<BVHNode>(tinySpheres));*/

	render(scene);

	// Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene);

	return (0);
	(void)argv;
}
