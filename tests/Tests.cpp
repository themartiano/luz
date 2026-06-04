#include "Color.hpp"
#include "Image.hpp"
#include "Materials/Lambertian.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "SceneFile/SceneFile.hpp"
#include "Hittables/BVHNode.hpp"
#include "Hittables/Sphere.hpp"
#include "Random.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	void	require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void	requireNear(double actual, double expected, const std::string& message)
	{
		require(std::abs(actual - expected) < 0.000001, message);
	}

	std::vector<unsigned char>	readFile(const std::filesystem::path& path)
	{
		std::ifstream stream(path, std::ios::binary);
		require(stream.good(), "Could not open test file: " + path.string());

		return (std::vector<unsigned char>(
			std::istreambuf_iterator<char>(stream),
			std::istreambuf_iterator<char>()
		));
	}

	void	requirePixelBytes(
		const std::vector<unsigned char>& bytes,
		std::size_t offset,
		unsigned char blue,
		unsigned char green,
		unsigned char red,
		const std::string& message
	)
	{
		require(bytes.size() >= offset + 3, "BMP test file is too small.");
		require(bytes[offset] == blue && bytes[offset + 1] == green && bytes[offset + 2] == red, message);
	}

	void	testColorMath(void)
	{
		Color color(0.25, 0.5, 0.75);
		Color result = (color + Color(0.25, 0.25, 0.25)) * 2.0;

		requireNear(result.getRed(), 1.0, "Color red channel math failed.");
		requireNear(result.getGreen(), 1.5, "Color green channel math failed.");
		requireNear(result.getBlue(), 2.0, "Color blue channel math failed.");
	}

	void	testNonSquareBMP(void)
	{
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_test_non_square";
		const std::filesystem::path bmpPath = outputPath.string() + ".bmp";

		Image image(3, 2);
		image.initialize();
		image.setPixel(0, 0, Color(1.0, 0.0, 0.0));
		image.setPixel(1, 0, Color(0.0, 1.0, 0.0));
		image.setPixel(2, 0, Color(0.0, 0.0, 1.0));
		image.setPixel(0, 1, Color(0.0, 1.0, 1.0));
		image.setPixel(1, 1, Color(1.0, 0.0, 1.0));
		image.setPixel(2, 1, Color(1.0, 1.0, 0.0));

		image.saveToBMP(outputPath.string());
		const std::vector<unsigned char> bytes = readFile(bmpPath);
		const std::size_t pixelDataOffset = 54;
		const std::size_t rowStride = 12;

		requirePixelBytes(bytes, pixelDataOffset + 0, 255, 255, 0, "BMP bottom-left pixel is wrong.");
		requirePixelBytes(bytes, pixelDataOffset + 3, 255, 0, 255, "BMP bottom-middle pixel is wrong.");
		requirePixelBytes(bytes, pixelDataOffset + 6, 0, 255, 255, "BMP bottom-right pixel is wrong.");
		requirePixelBytes(bytes, pixelDataOffset + rowStride + 0, 0, 0, 255, "BMP top-left pixel is wrong.");
		requirePixelBytes(bytes, pixelDataOffset + rowStride + 3, 0, 255, 0, "BMP top-middle pixel is wrong.");
		requirePixelBytes(bytes, pixelDataOffset + rowStride + 6, 255, 0, 0, "BMP top-right pixel is wrong.");

		std::filesystem::remove(bmpPath);
	}

	void	testSceneFileOutputName(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_output_test.luz";
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_custom_output";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< "resolution=2,2\n"
				<< "outputfilename=" << outputPath.string() << "\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());
		require(
			scene.getDefaultRenderOutputFileName() == outputPath.string() + ".bmp",
			"Scene outputfilename setting was not applied."
		);

		std::filesystem::remove(scenePath);
	}

	void	testSceneFileLoadsRelativeObject(void)
	{
		Scene scene;
		SceneFile::read(scene, "examples/scenes/demo.luz");

		require(scene.hasCamera(), "Demo scene did not load a camera.");
		require(!scene.getHittables().empty(), "Demo scene did not load any hittables.");
	}

	void	testBVHReturnsClosestHit(void)
	{
		std::vector<std::shared_ptr<Hittable>> hittables;
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		hittables.push_back(std::make_shared<Sphere>(Vector3(0.0, 0.0, -5.0), 1.0, material));
		hittables.push_back(std::make_shared<Sphere>(Vector3(0.0, 0.0, -2.0), 0.5, material));

		BVHNode bvh(hittables);
		Ray ray(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, -1.0));
		HitRecord hitRecord;

		require(bvh.hit(ray, hitRecord, 0.001, 100.0), "BVH did not report a hit.");
		requireNear(hitRecord.t0, 1.5, "BVH did not return the closest hit.");
	}

	void	testTinyRender(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(4);
		scene.getImage()->setHeight(3);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setRenderSky(SKY_NONE);
		scene.setBackgroundColor(Color(0.1, 0.2, 0.3));
		scene.setRenderingThreads(1);
		scene.addCamera(Camera(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0), 45, 0.0, 1.0));
		scene.addHittable(std::make_shared<Sphere>(
			Vector3(0.0, 0.0, -1.0),
			0.5,
			std::make_shared<Lambertian>(Color(0.8, 0.2, 0.2))
		));

		require(Renderer::render(scene), "Tiny render failed.");
		for (std::size_t y = 0; y < scene.getImage()->getHeight(); y++)
		{
			for (std::size_t x = 0; x < scene.getImage()->getWidth(); x++)
			{
				const Color pixel = scene.getImage()->getPixel(x, y);
				require(std::isfinite(pixel.getRed()), "Tiny render produced non-finite red value.");
				require(std::isfinite(pixel.getGreen()), "Tiny render produced non-finite green value.");
				require(std::isfinite(pixel.getBlue()), "Tiny render produced non-finite blue value.");
			}
		}
	}
}

int	main(void)
{
	try
	{
		testColorMath();
		testNonSquareBMP();
		testSceneFileOutputName();
		testSceneFileLoadsRelativeObject();
		testBVHReturnsClosestHit();
		testTinyRender();
	}
	catch (const std::exception& exception)
	{
		std::cerr << "Test failed: " << exception.what() << std::endl;
		return (1);
	}

	std::cout << "All tests passed." << std::endl;
	return (0);
}
