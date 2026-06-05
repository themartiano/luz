#include "Color.hpp"
#include "FlagsParser.hpp"
#include "Image.hpp"
#include "Materials/Lambertian.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "SceneFile/SceneFile.hpp"
#include "Hittables/BVHNode.hpp"
#include "Hittables/Sphere.hpp"
#include "Random.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	struct TiffTag
	{
		std::uint16_t	id;
		std::uint16_t	type;
		std::uint32_t	count;
		std::uint32_t	valueOrOffset;
	};

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

	template <typename Function>
	void	requireThrows(Function function, const std::string& message)
	{
		try
		{
			function();
		}
		catch (const std::exception&)
		{
			return;
		}

		throw std::runtime_error(message);
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

	void	requireRGBBytes(
		const std::vector<unsigned char>& bytes,
		std::size_t offset,
		unsigned char red,
		unsigned char green,
		unsigned char blue,
		const std::string& message
	)
	{
		require(bytes.size() >= offset + 3, "TIFF test file is too small.");
		require(bytes[offset] == red && bytes[offset + 1] == green && bytes[offset + 2] == blue, message);
	}

	std::uint16_t	readU16(const std::vector<unsigned char>& bytes, std::size_t offset)
	{
		require(bytes.size() >= offset + 2, "TIFF test file is too small.");
		return (
			static_cast<std::uint16_t>(bytes[offset]) |
			(static_cast<std::uint16_t>(bytes[offset + 1]) << 8)
		);
	}

	std::uint32_t	readU32(const std::vector<unsigned char>& bytes, std::size_t offset)
	{
		require(bytes.size() >= offset + 4, "TIFF test file is too small.");
		return (
			static_cast<std::uint32_t>(bytes[offset]) |
			(static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
			(static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
			(static_cast<std::uint32_t>(bytes[offset + 3]) << 24)
		);
	}

	TiffTag	readTiffTag(const std::vector<unsigned char>& bytes, std::size_t offset)
	{
		return (TiffTag{
			readU16(bytes, offset),
			readU16(bytes, offset + 2),
			readU32(bytes, offset + 4),
			readU32(bytes, offset + 8)
		});
	}

	TiffTag	findTiffTag(const std::vector<unsigned char>& bytes, std::size_t ifdOffset, std::uint16_t tagId)
	{
		const std::uint16_t tagCount = readU16(bytes, ifdOffset);

		for (std::uint16_t i = 0; i < tagCount; i++)
		{
			TiffTag tag = readTiffTag(bytes, ifdOffset + 2 + (i * 12));
			if (tag.id == tagId)
			{
				return (tag);
			}
		}

		throw std::runtime_error("TIFF tag not found.");
	}

	void	testColorMath(void)
	{
		Color color(0.25, 0.5, 0.75);
		Color result = (color + Color(0.25, 0.25, 0.25)) * 2.0;

		requireNear(result.getRed(), 1.0, "Color red channel math failed.");
		requireNear(result.getGreen(), 1.5, "Color green channel math failed.");
		requireNear(result.getBlue(), 2.0, "Color blue channel math failed.");
	}

	void	testToneMappingBlackPixel(void)
	{
		Image image(1, 1);
		image.initialize();
		image.setPixel(0, 0, Color(0.0, 0.0, 0.0));

		image.toneMap();
		const Color pixel = image.getPixel(0, 0);

		require(std::isfinite(pixel.getRed()), "Tone mapping black produced non-finite red value.");
		require(std::isfinite(pixel.getGreen()), "Tone mapping black produced non-finite green value.");
		require(std::isfinite(pixel.getBlue()), "Tone mapping black produced non-finite blue value.");
		requireNear(pixel.getRed(), 0.0, "Tone mapping black changed red value.");
		requireNear(pixel.getGreen(), 0.0, "Tone mapping black changed green value.");
		requireNear(pixel.getBlue(), 0.0, "Tone mapping black changed blue value.");
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

	void	testNonSquareTIFF(void)
	{
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_test_non_square";
		const std::filesystem::path tiffPath = outputPath.string() + ".tiff";

		Image image(3, 2);
		image.initialize();
		image.setPixel(0, 0, Color(1.0, 0.0, 0.0));
		image.setPixel(1, 0, Color(0.0, 1.0, 0.0));
		image.setPixel(2, 0, Color(0.0, 0.0, 1.0));
		image.setPixel(0, 1, Color(0.0, 1.0, 1.0));
		image.setPixel(1, 1, Color(1.0, 0.0, 1.0));
		image.setPixel(2, 1, Color(1.0, 1.0, 0.0));

		image.saveToTIFF(outputPath.string());
		const std::vector<unsigned char> bytes = readFile(tiffPath);

		require(bytes[0] == 'I' && bytes[1] == 'I', "TIFF byte order is wrong.");
		require(readU16(bytes, 2) == 42, "TIFF version is wrong.");

		const std::uint32_t ifdOffset = readU32(bytes, 4);
		require(findTiffTag(bytes, ifdOffset, 256).valueOrOffset == 3, "TIFF width is wrong.");
		require(findTiffTag(bytes, ifdOffset, 257).valueOrOffset == 2, "TIFF height is wrong.");

		const TiffTag bitsPerSample = findTiffTag(bytes, ifdOffset, 258);
		require(bitsPerSample.type == 3 && bitsPerSample.count == 3, "TIFF BitsPerSample tag is wrong.");
		require(readU16(bytes, bitsPerSample.valueOrOffset) == 8, "TIFF red bit depth is wrong.");
		require(readU16(bytes, bitsPerSample.valueOrOffset + 2) == 8, "TIFF green bit depth is wrong.");
		require(readU16(bytes, bitsPerSample.valueOrOffset + 4) == 8, "TIFF blue bit depth is wrong.");

		require(findTiffTag(bytes, ifdOffset, 259).valueOrOffset == 1, "TIFF compression is wrong.");
		require(findTiffTag(bytes, ifdOffset, 262).valueOrOffset == 2, "TIFF photometric interpretation is wrong.");
		require(findTiffTag(bytes, ifdOffset, 277).valueOrOffset == 3, "TIFF samples per pixel is wrong.");
		require(findTiffTag(bytes, ifdOffset, 278).valueOrOffset == 2, "TIFF rows per strip is wrong.");
		require(findTiffTag(bytes, ifdOffset, 279).valueOrOffset == 18, "TIFF strip byte count is wrong.");
		require(findTiffTag(bytes, ifdOffset, 284).valueOrOffset == 1, "TIFF planar configuration is wrong.");

		const std::uint32_t pixelDataOffset = findTiffTag(bytes, ifdOffset, 273).valueOrOffset;
		requireRGBBytes(bytes, pixelDataOffset + 0, 255, 0, 0, "TIFF top-left pixel is wrong.");
		requireRGBBytes(bytes, pixelDataOffset + 3, 0, 255, 0, "TIFF top-middle pixel is wrong.");
		requireRGBBytes(bytes, pixelDataOffset + 6, 0, 0, 255, "TIFF top-right pixel is wrong.");
		requireRGBBytes(bytes, pixelDataOffset + 9, 0, 255, 255, "TIFF bottom-left pixel is wrong.");
		requireRGBBytes(bytes, pixelDataOffset + 12, 255, 0, 255, "TIFF bottom-middle pixel is wrong.");
		requireRGBBytes(bytes, pixelDataOffset + 15, 255, 255, 0, "TIFF bottom-right pixel is wrong.");

		std::filesystem::remove(tiffPath);
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

	void	testSceneFileTiffOutputName(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_tiff_output_test.luz";
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_custom_output.tiff";
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
			scene.getDefaultRenderOutputFileName() == outputPath.string(),
			"Scene TIFF outputfilename setting was not preserved."
		);

		std::filesystem::remove(scenePath);
	}

	void	requireSceneFileSettingThrows(const std::string& settingLine, const std::string& message)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_invalid_setting_test.luz";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< settingLine << "\n\n";
		}

		bool threw = false;
		try
		{
			Scene scene;
			SceneFile::read(scene, scenePath.string());
		}
		catch (const std::exception&)
		{
			threw = true;
		}

		std::filesystem::remove(scenePath);
		require(threw, message);
	}

	void	testSceneFileRejectsInvalidSettings(void)
	{
		requireSceneFileSettingThrows("resolution=0,10", "Scene file accepted zero width.");
		requireSceneFileSettingThrows("samples=0", "Scene file accepted zero samples.");
		requireSceneFileSettingThrows("maxlightbounces=-1", "Scene file accepted negative max light bounces.");
		requireSceneFileSettingThrows("gamma=2", "Scene file accepted non-binary gamma.");
		requireSceneFileSettingThrows("bloom=2", "Scene file accepted non-binary bloom.");
		requireSceneFileSettingThrows("sky=wat", "Scene file accepted unknown sky setting.");
		requireSceneFileSettingThrows("atmosphere=0,1,2,3,4,0,1,0", "Scene file accepted zero atmosphere samples.");
	}

	void	testSceneFileLoadsRelativeObject(void)
	{
		Scene scene;
		SceneFile::read(scene, "examples/scenes/demo.luz");

		require(scene.hasCamera(), "Demo scene did not load a camera.");
		require(!scene.getHittables().empty(), "Demo scene did not load any hittables.");
	}

	void	testSceneFileLoadsTransformedObject(void)
	{
		const std::filesystem::path directory = std::filesystem::temp_directory_path();
		const std::filesystem::path scenePath = directory / "luz_obj_transform_test.luz";
		const std::filesystem::path objectPath = directory / "luz_obj_transform_test.obj";
		{
			std::ofstream objectStream(objectPath);
			objectStream
				<< "v 0.0 0.0 0.0\n"
				<< "v 1.0 0.0 0.0\n"
				<< "v 0.0 1.0 0.0\n"
				<< "f 1 2 3\n";
		}
		{
			std::ofstream sceneStream(scenePath);
			sceneStream
				<< "[scene]\n"
				<< "objects{\n"
				<< "obj=" << objectPath.filename().string() << ",(1.0,2.0,3.0),material[\n"
				<< "metal=(0.8,0.7,0.6),0.2\n"
				<< "]\n"
				<< "}\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.getHittables().size() == 1, "Transformed OBJ scene did not load one hittable.");
		require(scene.getHittables()[0]->getMaterial()->getType() == METAL, "Transformed OBJ material was not applied.");

		AABB boundingBox;
		require(scene.getHittables()[0]->createBoundingBox(boundingBox), "Transformed OBJ did not create a bounding box.");
		require(boundingBox.getMinimum().getX() > 0.99 && boundingBox.getMinimum().getX() < 1.0, "Transformed OBJ X offset was not applied.");
		require(boundingBox.getMinimum().getY() > 1.99 && boundingBox.getMinimum().getY() < 2.0, "Transformed OBJ Y offset was not applied.");
		require(boundingBox.getMinimum().getZ() > 2.99 && boundingBox.getMinimum().getZ() < 3.0, "Transformed OBJ Z offset was not applied.");

		std::filesystem::remove(scenePath);
		std::filesystem::remove(objectPath);
	}

	void	testSceneFileRejectsInvalidMaterial(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_invalid_material_test.luz";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< "resolution=2,2\n\n"
				<< "[scene]\n"
				<< "camera=(0,0,1),(0,0,-1),45,0,1\n"
				<< "objects{\n"
				<< "sphere=(0,0,-1),0.5,material[\n"
				<< "plastic=(1.0,1.0,1.0)\n"
				<< "]\n"
				<< "}\n";
		}

		Scene scene;
		requireThrows(
			[&scene, &scenePath]()
			{
				SceneFile::read(scene, scenePath.string());
			},
			"Scene file accepted an unknown material."
		);

		std::filesystem::remove(scenePath);
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

	void	requireFlagParseThrows(std::vector<std::string> arguments, const std::string& message)
	{
		std::vector<char*> argv;
		argv.push_back(const_cast<char*>("Luz"));
		for (std::string& argument : arguments)
		{
			argv.push_back(argument.data());
		}

		requireThrows([&]()
		{
			Scene scene;
			FlagsParser(static_cast<int>(argv.size()), argv.data()).parse(scene);
		}, message);
	}

	void	testFlagsRejectInvalidValues(void)
	{
		requireFlagParseThrows({"--samples", "0"}, "CLI accepted zero samples.");
		requireFlagParseThrows({"--maxLightBounces", "-1"}, "CLI accepted negative max light bounces.");
		requireFlagParseThrows({"--resolution", "-1x100"}, "CLI accepted negative width.");
		requireFlagParseThrows({"--threads", "0"}, "CLI accepted zero threads.");
	}

	void	testSettersRejectInvalidValues(void)
	{
		Scene scene;
		Image image;

		requireThrows([&]() { scene.setSampleCount(0); }, "Scene accepted zero samples.");
		requireThrows([&]() { scene.setMaxLightBounces(-1); }, "Scene accepted negative max light bounces.");
		requireThrows([&]() { scene.setRenderingThreads(0); }, "Scene accepted zero rendering threads.");
		requireThrows([&]() { image.setWidth(0); }, "Image accepted zero width.");
		requireThrows([&]() { image.setHeight(0); }, "Image accepted zero height.");
		requireThrows([&]() { Atmosphere().setSamples(0); }, "Atmosphere accepted zero samples.");
		requireThrows([&]() { Atmosphere().setLightSamples(0); }, "Atmosphere accepted zero light samples.");
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
		testToneMappingBlackPixel();
		testNonSquareBMP();
		testNonSquareTIFF();
		testSceneFileOutputName();
		testSceneFileTiffOutputName();
		testSceneFileRejectsInvalidSettings();
		testSceneFileLoadsRelativeObject();
		testSceneFileLoadsTransformedObject();
		testSceneFileRejectsInvalidMaterial();
		testBVHReturnsClosestHit();
		testFlagsRejectInvalidValues();
		testSettersRejectInvalidValues();
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
