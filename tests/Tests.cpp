#include "AABB.hpp"
#include "Color.hpp"
#include "Denoise/NonLocalMeans.hpp"
#include "FlagsParser.hpp"
#include "Image.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Principled.hpp"
#include "OBJReader.hpp"
#include "PDFs/HittablePDF.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "SceneFile/SceneFile.hpp"
#include "Hittables/BVHNode.hpp"
#include "Hittables/Cube.hpp"
#include "Hittables/Mesh.hpp"
#include "Hittables/Rectangle.hpp"
#include "Hittables/Sphere.hpp"
#include "Hittables/Triangle.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include "Transform.hpp"
#include "Utilities.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
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

	void	requireVectorNear(const Vector3& actual, const Vector3& expected, const std::string& message)
	{
		requireNear(actual.getX(), expected.getX(), message + " X component is wrong.");
		requireNear(actual.getY(), expected.getY(), message + " Y component is wrong.");
		requireNear(actual.getZ(), expected.getZ(), message + " Z component is wrong.");
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

	void	testNonLocalMeansReducesImpulseAndPreservesEdge(void)
	{
		Image image(7, 5);
		image.initialize();

		for (std::size_t y = 0; y < image.getHeight(); y++)
		{
			for (std::size_t x = 0; x < image.getWidth(); x++)
			{
				image.setPixel(x, y, x < 3 ? Color(0.0, 0.0, 0.0) : Color(1.0, 1.0, 1.0));
			}
		}
		image.setPixel(1, 2, Color(1.0, 1.0, 1.0));

		Denoise::NonLocalMeansSettings settings;
		settings.searchRadius = 2;
		settings.patchRadius = 1;
		settings.strength = 0.3;
		Denoise::apply(image, settings);

		require(image.getPixel(1, 2).getRed() < 0.75, "Denoise did not reduce an isolated impulse.");
		require(image.getPixel(2, 2).getRed() < 0.25, "Denoise blurred too much color across the dark side of an edge.");
		require(image.getPixel(3, 2).getRed() > 0.75, "Denoise blurred too much color across the bright side of an edge.");
	}

	void	testTerminalFilePath(void)
	{
		require(
			Utilities::terminalFilePath("named_blocks.bmp") == "./named_blocks.bmp",
			"Terminal file path did not prefix bare relative output."
		);
		require(
			Utilities::terminalFilePath("exports/render.bmp") == "./exports/render.bmp",
			"Terminal file path did not prefix nested relative output."
		);
		require(
			Utilities::terminalFilePath("./named_blocks.bmp") == "./named_blocks.bmp",
			"Terminal file path changed an explicit current-directory path."
		);
		require(
			Utilities::terminalFilePath("../named_blocks.bmp") == "../named_blocks.bmp",
			"Terminal file path changed a parent-directory path."
		);

		const std::string absolutePath = (std::filesystem::temp_directory_path() / "named_blocks.bmp").string();
		require(
			Utilities::terminalFilePath(absolutePath) == absolutePath,
			"Terminal file path changed an absolute path."
		);
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

	void	testSceneFileDenoiseOutputName(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_denoise_output_test.luz";
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_custom_denoised.tiff";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< "resolution=2,2\n"
				<< "denoise=1\n"
				<< "denoiseoutputfilename=" << outputPath.string() << "\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());
		require(scene.getDenoise(), "Scene denoise setting was not applied.");
		require(
			scene.getDenoiseOutputFileName() == outputPath.string(),
			"Scene denoise output setting was not applied."
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
		requireSceneFileSettingThrows("denoise=2", "Scene file accepted non-binary denoise.");
		requireSceneFileSettingThrows("denoiseoutputfilename=", "Scene file accepted empty denoise output name.");
		requireSceneFileSettingThrows("sky=wat", "Scene file accepted unknown sky setting.");
		requireSceneFileSettingThrows("atmosphere=0,1,2,3,4,0,1,0", "Scene file accepted zero atmosphere samples.");
	}

	void	testSceneFileBackgroundSetting(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_background_setting_test.luz";
		{
			std::ofstream sceneStream(scenePath);
			sceneStream
				<< "[settings]\n"
				<< "sky=none\n"
				<< "background=(0.1,0.2,0.3)\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		const Color background = scene.getBackgroundColor();
		requireNear(background.getRed(), 0.1, "Background red component was not parsed.");
		requireNear(background.getGreen(), 0.2, "Background green component was not parsed.");
		requireNear(background.getBlue(), 0.3, "Background blue component was not parsed.");

		std::filesystem::remove(scenePath);
	}

	void	testSceneFileLegacyCameraPreservesDecimalFOV(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_legacy_camera_fov_test.luz";
		{
			std::ofstream sceneStream(scenePath);
			sceneStream
				<< "[scene]\n"
				<< "camera=(0,0,1),(0,0,-1),35.489342,0,1\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.hasCamera(), "Legacy camera scene did not load a camera.");
		requireNear(scene.getActiveCamera().getFOV(), 35.489342, "Legacy camera FOV lost decimal precision.");

		std::filesystem::remove(scenePath);
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

	void	testSceneFileLoadsNamedBlocks(void)
	{
		const std::filesystem::path directory = std::filesystem::temp_directory_path();
		const std::filesystem::path scenePath = directory / "luz_named_blocks_test.luz";
		const std::filesystem::path objectPath = directory / "luz_named_blocks_test.obj";
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
				<< "[settings]\n"
				<< "resolution=2,2\n\n"
				<< "[materials]\n"
				<< "material shiny {\n"
				<< "type=principled\n"
				<< "base_color=(0.9,0.8,0.7)\n"
				<< "metallic=1\n"
				<< "roughness=0.2\n"
				<< "}\n\n"
				<< "[meshes]\n"
				<< "mesh triangle_mesh {\n"
				<< "file=" << objectPath.filename().string() << "\n"
				<< "}\n\n"
				<< "[scene]\n"
				<< "camera main {\n"
				<< "position=(0,0,5)\n"
				<< "direction=(0,0,-1)\n"
				<< "up=(0,1,0)\n"
				<< "fov=45\n"
				<< "aperture=0\n"
				<< "focusDistance=5\n"
				<< "}\n"
				<< "object triangle {\n"
				<< "mesh=triangle_mesh\n"
				<< "position=(1,2,3)\n"
				<< "rotation=(0,0,90)\n"
				<< "scale=(2,1,1)\n"
				<< "material=shiny\n"
				<< "}\n"
				<< "area_light key {\n"
				<< "position=(0,4,0)\n"
				<< "normal=(0,-1,0)\n"
				<< "size=(2,3)\n"
				<< "color=(1,1,1)\n"
				<< "intensity=4\n"
				<< "}\n"
				<< "point_light fill {\n"
				<< "position=(2,2,2)\n"
				<< "radius=0.25\n"
				<< "color=(0.5,0.6,1.0)\n"
				<< "intensity=1.5\n"
				<< "}\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.hasCamera(), "Named-block scene did not load a camera.");
		requireNear(scene.getActiveCamera().getUpDirection().getY(), 1.0, "Named-block camera up vector was not parsed.");
		require(scene.getHittables().size() == 3, "Named-block scene did not load object and lights.");
		require(scene.getHittables()[0]->getMaterial()->getType() == METAL, "Principled material did not map to metal.");
		require(scene.getHittables()[1]->getMaterial()->getType() == EMISSIVE, "Area light did not create an emissive hittable.");
		require(scene.getHittables()[2]->getMaterial()->getType() == EMISSIVE, "Point light did not create an emissive hittable.");

		scene.updateLights();
		require(scene.getLights().size() == 2, "Named-block lights were not registered as lights.");

		AABB boundingBox;
		require(scene.getHittables()[0]->createBoundingBox(boundingBox), "Named-block object did not create a bounding box.");
		require(boundingBox.getMinimum().getX() > -0.01 && boundingBox.getMinimum().getX() < 0.01, "Named-block object rotation X minimum is wrong.");
		require(boundingBox.getMaximum().getY() > 3.99 && boundingBox.getMaximum().getY() < 4.01, "Named-block object rotation Y maximum is wrong.");
		require(boundingBox.getMinimum().getZ() > 2.99 && boundingBox.getMinimum().getZ() < 3.0, "Named-block object Z offset is wrong.");

		std::filesystem::remove(scenePath);
		std::filesystem::remove(objectPath);
	}

	void	testSceneFileLoadsNonMetallicPrincipledMaterial(void)
	{
		const std::filesystem::path directory = std::filesystem::temp_directory_path();
		const std::filesystem::path scenePath = directory / "luz_principled_material_test.luz";
		const std::filesystem::path objectPath = directory / "luz_principled_material_test.obj";
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
				<< "[settings]\n"
				<< "resolution=2,2\n\n"
				<< "[materials]\n"
				<< "material plastic {\n"
				<< "type=principled\n"
				<< "base_color=(0.8,0.8,0.75)\n"
				<< "metallic=0\n"
				<< "roughness=0.35\n"
				<< "}\n\n"
				<< "[meshes]\n"
				<< "mesh triangle_mesh {\n"
				<< "file=" << objectPath.filename().string() << "\n"
				<< "}\n\n"
				<< "[scene]\n"
				<< "camera main {\n"
				<< "position=(0,0,3)\n"
				<< "direction=(0,0,-1)\n"
				<< "fov=45\n"
				<< "aperture=0\n"
				<< "focusDistance=3\n"
				<< "}\n"
				<< "object triangle {\n"
				<< "mesh=triangle_mesh\n"
				<< "position=(0,0,-1)\n"
				<< "rotation=(0,0,0)\n"
				<< "scale=(1,1,1)\n"
				<< "material=plastic\n"
				<< "}\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.getHittables().size() == 1, "Principled scene did not load the sphere.");
		require(scene.getHittables()[0]->getMaterial()->getType() == PRINCIPLED, "Non-metallic principled material did not remain principled.");

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

	void	testTinyTriangleHitAndNormal(void)
	{
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		Triangle triangle(
			Vector3(0.0, 0.0, -1.0),
			Vector3(0.0001, 0.0, -1.0),
			Vector3(0.0, 0.0001, -1.0),
			material
		);
		Ray ray(Vector3(0.000025, 0.000025, 0.0), Vector3(0.0, 0.0, -1.0));
		HitRecord hitRecord;

		require(triangle.hit(ray, hitRecord, 0.001, 100.0), "Tiny valid triangle was rejected.");
		requireNear(hitRecord.t0, 1.0, "Tiny triangle returned the wrong hit distance.");
		requireNear(Utilities::vectorLength(hitRecord.normal), 1.0, "Triangle hit normal was not normalized.");
		require(hitRecord.normal.getZ() > 0.0, "Triangle hit normal was not oriented against the ray.");
	}

	void	testTrianglePDFAndRandomSampling(void)
	{
		setRandomSeed(123);

		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		Triangle triangle(
			Vector3(0.0, 0.0, -1.0),
			Vector3(1.0, 0.0, -1.0),
			Vector3(0.0, 1.0, -1.0),
			material
		);
		const Vector3 origin(0.25, 0.25, 0.0);
		const Vector3 direction(0.0, 0.0, -1.0);

		requireNear(triangle.area(), 0.5, "Triangle area is wrong.");
		requireNear(triangle.pdfValue(origin, direction), 2.0, "Triangle PDF is wrong.");

		for (int i = 0; i < 8; i++)
		{
			Ray ray(origin, triangle.random(origin));
			HitRecord hitRecord;

			require(triangle.hit(ray, hitRecord, 0.001, 100.0), "Triangle generated a random direction that missed.");
		}
	}

	void	testTriangleInterpolatesVertexNormals(void)
	{
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		Triangle triangle(
			Vector3(0.0, 0.0, -1.0),
			Vector3(1.0, 0.0, -1.0),
			Vector3(0.0, 1.0, -1.0),
			Vector3(0.0, 1.0, 1.0),
			Vector3(0.0, 1.0, 1.0),
			Vector3(0.0, 1.0, 1.0),
			material
		);
		Ray ray(Vector3(0.25, 0.25, 0.0), Vector3(0.0, 0.0, -1.0));
		HitRecord hitRecord;

		require(triangle.hit(ray, hitRecord, 0.001, 100.0), "Triangle with vertex normals was not hit.");
		require(hitRecord.normal.getY() > 0.6, "Triangle did not use interpolated vertex normal Y.");
		require(hitRecord.normal.getZ() > 0.6, "Triangle did not use interpolated vertex normal Z.");
	}

	void	testOBJReaderLoadsVertexNormals(void)
	{
		const std::filesystem::path objectPath = std::filesystem::temp_directory_path() / "luz_obj_normals_test.obj";
		{
			std::ofstream objectStream(objectPath);
			objectStream
				<< "v 0.0 0.0 -1.0\n"
				<< "v 1.0 0.0 -1.0\n"
				<< "v 0.0 1.0 -1.0\n"
				<< "vn 0.0 1.0 1.0\n"
				<< "vn 0.0 1.0 1.0\n"
				<< "vn 0.0 1.0 1.0\n"
				<< "f 1//1 2//2 3//3\n";
		}

		ObjReadOptions options;
		options.quiet = true;
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		Mesh mesh = readObj(
			objectPath.string(),
			Vector3(0.0, 0.0, 0.0),
			Vector3(0.0, 0.0, 0.0),
			Vector3(1.0, 1.0, 1.0),
			material,
			options
		);
		Ray ray(Vector3(0.25, 0.25, 0.0), Vector3(0.0, 0.0, -1.0));
		HitRecord hitRecord;

		require(mesh.hit(ray, hitRecord, 0.001, 100.0), "OBJ mesh with vertex normals was not hit.");
		require(hitRecord.normal.getY() > 0.6, "OBJ reader did not preserve vertex normal Y.");
		require(hitRecord.normal.getZ() > 0.6, "OBJ reader did not preserve vertex normal Z.");

		std::filesystem::remove(objectPath);
	}

	void	testMeshPDFAndRandomSampling(void)
	{
		setRandomSeed(123);

		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		std::vector<std::shared_ptr<Hittable>> triangles;
		triangles.push_back(std::make_shared<Triangle>(
			Vector3(0.0, 0.0, -1.0),
			Vector3(1.0, 0.0, -1.0),
			Vector3(0.0, 1.0, -1.0),
			material
		));
		triangles.push_back(std::make_shared<Triangle>(
			Vector3(2.0, 0.0, -1.0),
			Vector3(4.0, 0.0, -1.0),
			Vector3(2.0, 1.0, -1.0),
			material
		));
		Mesh mesh(Vector3(), material, triangles);
		const Vector3 origin(0.25, 0.25, 0.0);
		const Vector3 direction(0.0, 0.0, -1.0);

		requireNear(mesh.pdfValue(origin, direction), 2.0 / 3.0, "Mesh PDF did not use total mesh area.");

		for (int i = 0; i < 8; i++)
		{
			Ray ray(origin, mesh.random(origin));
			HitRecord hitRecord;

			require(mesh.hit(ray, hitRecord, 0.001, 100.0), "Mesh generated a random direction that missed.");
		}
	}

	void	testAABBDefaultBounds(void)
	{
		const AABB boundingBox;

		requireVectorNear(boundingBox.getMinimum(), Vector3(0.0, 0.0, 0.0), "Default AABB minimum");
		requireVectorNear(boundingBox.getMaximum(), Vector3(0.0, 0.0, 0.0), "Default AABB maximum");
	}

	void	testRectangleBoundingBoxes(void)
	{
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		AABB boundingBox;

		Rectangle yRectangle(
			Transform(Vector3(1.0, 2.0, 3.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);
		require(yRectangle.createBoundingBox(boundingBox), "Y-aligned rectangle did not create a bounding box.");
		requireVectorNear(boundingBox.getMinimum(), Vector3(-1.0, 2.0 - T_MIN, 0.0), "Y-aligned rectangle minimum");
		requireVectorNear(boundingBox.getMaximum(), Vector3(3.0, 2.0 + T_MIN, 6.0), "Y-aligned rectangle maximum");

		Rectangle zRectangle(
			Transform(Vector3(1.0, 2.0, 3.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);
		require(zRectangle.createBoundingBox(boundingBox), "Z-aligned rectangle did not create a bounding box.");
		requireVectorNear(boundingBox.getMinimum(), Vector3(-1.0, -1.0, 3.0 - T_MIN), "Z-aligned rectangle minimum");
		requireVectorNear(boundingBox.getMaximum(), Vector3(3.0, 5.0, 3.0 + T_MIN), "Z-aligned rectangle maximum");

		Rectangle xRectangle(
			Transform(Vector3(1.0, 2.0, 3.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);
		require(xRectangle.createBoundingBox(boundingBox), "X-aligned rectangle did not create a bounding box.");
		requireVectorNear(boundingBox.getMinimum(), Vector3(1.0 - T_MIN, -1.0, 1.0), "X-aligned rectangle minimum");
		requireVectorNear(boundingBox.getMaximum(), Vector3(1.0 + T_MIN, 5.0, 5.0), "X-aligned rectangle maximum");

		Rectangle zeroNormalRectangle(
			Transform(Vector3(1.0, 2.0, 3.0), Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);
		require(!zeroNormalRectangle.createBoundingBox(boundingBox), "Zero-normal rectangle created a bounding box.");
	}

	void	requireRectangleRandomSamplesHit(Rectangle& rectangle, const Vector3& origin, const std::string& message)
	{
		for (int i = 0; i < 8; i++)
		{
			Ray ray(origin, rectangle.random(origin));
			HitRecord hitRecord;

			require(rectangle.hit(ray, hitRecord, 0.001, 100.0), message);
		}
	}

	void	testRectangleRandomSamplesSupportedAxes(void)
	{
		setRandomSeed(123);

		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		Rectangle yRectangle(
			Transform(Vector3(0.0, 5.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);
		Rectangle zRectangle(
			Transform(Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);
		Rectangle xRectangle(
			Transform(Vector3(5.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)),
			4.0,
			6.0,
			material
		);

		requireRectangleRandomSamplesHit(yRectangle, Vector3(0.0, 0.0, 0.0), "Y-aligned rectangle generated a direction that missed.");
		requireRectangleRandomSamplesHit(zRectangle, Vector3(0.0, 0.0, 0.0), "Z-aligned rectangle generated a direction that missed.");
		requireRectangleRandomSamplesHit(xRectangle, Vector3(0.0, 0.0, 0.0), "X-aligned rectangle generated a direction that missed.");
	}

	void	testCubeBoundingBoxAndSetters(void)
	{
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		Cube cube;
		cube.setTransform(Transform(Vector3(10.0, 20.0, 30.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)));
		cube.setWidth(4.0);
		cube.setHeight(6.0);
		cube.setDepth(8.0);
		cube.setMaterial(material);

		AABB boundingBox;
		require(cube.createBoundingBox(boundingBox), "Cube did not create a bounding box.");
		requireVectorNear(boundingBox.getMinimum(), Vector3(8.0, 17.0, 26.0), "Cube minimum");
		requireVectorNear(boundingBox.getMaximum(), Vector3(12.0, 23.0, 34.0), "Cube maximum");

		HitRecord hitRecord;
		Ray backFaceRay(Vector3(10.0, 20.0, 40.0), Vector3(0.0, 0.0, -1.0));
		require(cube.hit(backFaceRay, hitRecord, 0.001, 100.0), "Cube setters did not regenerate back face geometry.");
		requireNear(hitRecord.t0, 6.0, "Cube back face hit distance is wrong.");

		Ray rightFaceRay(Vector3(20.0, 20.0, 34.0), Vector3(-1.0, 0.0, 0.0));
		require(cube.hit(rightFaceRay, hitRecord, 0.001, 100.0), "Cube side face did not span the cube depth.");
		requireNear(hitRecord.t0, 8.0, "Cube side face hit distance is wrong.");
	}

	void	testHittablePDFAveragesMultipleLights(void)
	{
		auto material = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.8));
		auto visibleLight = std::make_shared<Sphere>(Vector3(0.0, 0.0, -2.0), 0.5, material);
		auto offAxisLight = std::make_shared<Sphere>(Vector3(10.0, 0.0, -2.0), 0.5, material);
		const Vector3 origin(0.0, 0.0, 0.0);
		const Vector3 direction(0.0, 0.0, -1.0);

		HittablePDF singleLightPDF(visibleLight, origin);
		HittablePDF multiLightPDF(
			std::vector<std::shared_ptr<Hittable>>{visibleLight, offAxisLight},
			origin
		);

		requireNear(
			multiLightPDF.value(direction),
			singleLightPDF.value(direction) / 2.0,
			"Multi-light HittablePDF did not average light densities."
		);
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

	std::unique_ptr<Scene>	parseFlags(std::vector<std::string> arguments)
	{
		std::vector<char*> argv;
		argv.push_back(const_cast<char*>("Luz"));
		for (std::string& argument : arguments)
		{
			argv.push_back(argument.data());
		}

		auto scene = std::make_unique<Scene>();
		FlagsParser(static_cast<int>(argv.size()), argv.data()).parse(*scene);
		return (scene);
	}

	void	testFlagsParseDenoiseOptions(void)
	{
		std::unique_ptr<Scene> enabledScene = parseFlags({"--denoise"});
		require(enabledScene->getDenoise(), "Bare --denoise did not enable denoising.");

		std::unique_ptr<Scene> falseScene = parseFlags({"--denoise", "false"});
		require(!falseScene->getDenoise(), "--denoise false did not disable denoising.");

		std::unique_ptr<Scene> disabledScene = parseFlags({"--denoise", "--no-denoise"});
		require(!disabledScene->getDenoise(), "--no-denoise did not override --denoise.");

		std::unique_ptr<Scene> outputScene = parseFlags({"--denoise-output", "custom_denoised.tiff"});
		require(
			outputScene->getDenoiseOutputFileName() == "custom_denoised.tiff",
			"--denoise-output was not parsed."
		);
	}

	void	testFlagsRejectInvalidValues(void)
	{
		requireFlagParseThrows({"--samples", "0"}, "CLI accepted zero samples.");
		requireFlagParseThrows({"--maxLightBounces", "-1"}, "CLI accepted negative max light bounces.");
		requireFlagParseThrows({"--resolution", "-1x100"}, "CLI accepted negative width.");
		requireFlagParseThrows({"--threads", "0"}, "CLI accepted zero threads.");
		requireFlagParseThrows({"--denoise", "maybe"}, "CLI accepted invalid denoise value.");
		requireFlagParseThrows({"--denoise-output"}, "CLI accepted missing denoise output path.");
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

	void	testTinyDenoisedRenderProducesCompanionImage(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setDenoise(true);
		scene.setRenderSky(SKY_NONE);
		scene.setBackgroundColor(Color(0.1, 0.2, 0.3));
		scene.setRenderingThreads(1);
		scene.addCamera(Camera(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0), 45, 0.0, 1.0));

		require(Renderer::render(scene), "Tiny denoised render failed.");
		require(scene.hasDenoisedImage(), "Denoised render did not create a companion image.");
		require(scene.getDenoisedImage()->getWidth() == scene.getImage()->getWidth(), "Denoised image width is wrong.");
		require(scene.getDenoisedImage()->getHeight() == scene.getImage()->getHeight(), "Denoised image height is wrong.");
		requireNear(scene.getImage()->getPixel(0, 0).getRed(), 0.1, "Original render image was changed by denoising.");
		requireNear(scene.getDenoisedImage()->getPixel(0, 0).getRed(), 0.1, "Denoised companion image value is wrong.");
	}

	void	testZeroFocusDistanceRender(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setRenderSky(SKY_NONE);
		scene.setBackgroundColor(Color(0.1, 0.2, 0.3));
		scene.setRenderingThreads(1);
		scene.addCamera(Camera(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0), 45, 3.0, 0.0));

		require(Renderer::render(scene), "Zero-focus render failed.");
		for (std::size_t y = 0; y < scene.getImage()->getHeight(); y++)
		{
			for (std::size_t x = 0; x < scene.getImage()->getWidth(); x++)
			{
				const Color pixel = scene.getImage()->getPixel(x, y);
				require(std::isfinite(pixel.getRed()), "Zero-focus render produced non-finite red value.");
				require(std::isfinite(pixel.getGreen()), "Zero-focus render produced non-finite green value.");
				require(std::isfinite(pixel.getBlue()), "Zero-focus render produced non-finite blue value.");
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
		testNonLocalMeansReducesImpulseAndPreservesEdge();
		testTerminalFilePath();
		testNonSquareBMP();
		testNonSquareTIFF();
		testSceneFileOutputName();
		testSceneFileTiffOutputName();
		testSceneFileDenoiseOutputName();
		testSceneFileRejectsInvalidSettings();
		testSceneFileBackgroundSetting();
		testSceneFileLegacyCameraPreservesDecimalFOV();
		testSceneFileLoadsRelativeObject();
		testSceneFileLoadsTransformedObject();
		testSceneFileLoadsNamedBlocks();
		testSceneFileLoadsNonMetallicPrincipledMaterial();
		testSceneFileRejectsInvalidMaterial();
		testBVHReturnsClosestHit();
		testTinyTriangleHitAndNormal();
		testTrianglePDFAndRandomSampling();
		testTriangleInterpolatesVertexNormals();
		testOBJReaderLoadsVertexNormals();
		testMeshPDFAndRandomSampling();
		testAABBDefaultBounds();
		testRectangleBoundingBoxes();
		testRectangleRandomSamplesSupportedAxes();
		testCubeBoundingBoxAndSetters();
		testHittablePDFAveragesMultipleLights();
		testFlagsParseDenoiseOptions();
		testFlagsRejectInvalidValues();
		testSettersRejectInvalidValues();
		testTinyRender();
		testTinyDenoisedRenderProducesCompanionImage();
		testZeroFocusDistanceRender();
	}
	catch (const std::exception& exception)
	{
		std::cerr << "Test failed: " << exception.what() << std::endl;
		return (1);
	}

	std::cout << "All tests passed." << std::endl;
	return (0);
}
