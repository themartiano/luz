#include "AABB.hpp"
#include "Color.hpp"
#include "EnvironmentMap.hpp"
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
#include "Blur/Gaussian.hpp"
#include "Defaults.hpp"
#include "Random.hpp"
#include "Transform.hpp"
#include "Utilities.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
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

	std::uint32_t	readBE32(const std::vector<unsigned char>& bytes, std::size_t offset)
	{
		require(bytes.size() >= offset + 4, "PNG test file is too small.");
		return (
			(static_cast<std::uint32_t>(bytes[offset]) << 24) |
			(static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
			(static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
			static_cast<std::uint32_t>(bytes[offset + 3])
		);
	}

	std::uint32_t	adler32(const std::vector<unsigned char>& data)
	{
		constexpr std::uint32_t MOD_ADLER = 65521;
		std::uint32_t a = 1;
		std::uint32_t b = 0;

		for (unsigned char byte : data)
		{
			a = (a + byte) % MOD_ADLER;
			b = (b + a) % MOD_ADLER;
		}

		return ((b << 16) | a);
	}

	std::vector<unsigned char>	inflatePNGZlib(const std::vector<unsigned char>& zlibData)
	{
		require(zlibData.size() >= 6, "PNG zlib stream is too small.");

		const std::uint16_t header = static_cast<std::uint16_t>((zlibData[0] << 8) | zlibData[1]);
		require((zlibData[0] & 0x0f) == 8, "PNG zlib stream is not deflate.");
		require((header % 31) == 0, "PNG zlib header check bits are wrong.");

		std::vector<unsigned char> output;
		std::size_t offset = 2;
		const std::size_t end = zlibData.size() - 4;
		bool finalBlock = false;

		while (!finalBlock)
		{
			require(offset + 5 <= end, "PNG stored deflate stream ended early.");
			const unsigned char blockHeader = zlibData[offset++];
			finalBlock = (blockHeader & 1) != 0;

			require(((blockHeader >> 1) & 3) == 0, "PNG deflate block is not stored.");
			require((blockHeader & 0xf8) == 0, "PNG stored deflate padding bits are not zero.");

			const std::uint16_t length = static_cast<std::uint16_t>(
				zlibData[offset] | (zlibData[offset + 1] << 8)
			);
			const std::uint16_t inverseLength = static_cast<std::uint16_t>(
				zlibData[offset + 2] | (zlibData[offset + 3] << 8)
			);
			offset += 4;

			require(static_cast<std::uint16_t>(~length) == inverseLength, "PNG stored deflate length check failed.");
			require(offset + length <= end, "PNG stored deflate data ended early.");
			output.insert(output.end(), zlibData.begin() + offset, zlibData.begin() + offset + length);
			offset += length;
		}

		require(offset == end, "PNG zlib stream has trailing deflate data.");
		require(readBE32(zlibData, zlibData.size() - 4) == adler32(output), "PNG zlib Adler-32 is wrong.");
		return (output);
	}

	float	readF32(const std::vector<unsigned char>& bytes, std::size_t offset)
	{
		const std::uint32_t bits = readU32(bytes, offset);
		float value;

		std::memcpy(&value, &bits, sizeof(value));
		return (value);
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

	struct PNGData
	{
		std::uint32_t				width = 0;
		std::uint32_t				height = 0;
		unsigned char				bitDepth = 0;
		unsigned char				colorType = 0;
		std::vector<unsigned char>	imageData;
	};

	PNGData	readPNG(const std::filesystem::path& path)
	{
		const std::vector<unsigned char> bytes = readFile(path);
		const unsigned char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
		PNGData png;
		std::size_t offset = 8;
		bool sawIHDR = false;
		bool sawIEND = false;

		require(bytes.size() >= 8, "PNG test file is too small.");
		for (std::size_t i = 0; i < 8; i++)
		{
			require(bytes[i] == signature[i], "PNG signature is wrong.");
		}

		while (offset < bytes.size())
		{
			const std::uint32_t length = readBE32(bytes, offset);
			const std::size_t typeOffset = offset + 4;
			const std::size_t dataOffset = typeOffset + 4;
			const std::size_t crcOffset = dataOffset + length;

			require(crcOffset + 4 <= bytes.size(), "PNG chunk extends past end of file.");
			const std::string type(reinterpret_cast<const char*>(&bytes[typeOffset]), 4);
			if (type == "IHDR")
			{
				require(length == 13, "PNG IHDR chunk length is wrong.");
				png.width = readBE32(bytes, dataOffset);
				png.height = readBE32(bytes, dataOffset + 4);
				png.bitDepth = bytes[dataOffset + 8];
				png.colorType = bytes[dataOffset + 9];
				require(bytes[dataOffset + 10] == 0, "PNG compression method is wrong.");
				require(bytes[dataOffset + 11] == 0, "PNG filter method is wrong.");
				require(bytes[dataOffset + 12] == 0, "PNG interlace method is wrong.");
				sawIHDR = true;
			}
			else if (type == "IDAT")
			{
				png.imageData.insert(png.imageData.end(), bytes.begin() + dataOffset, bytes.begin() + crcOffset);
			}
			else if (type == "IEND")
			{
				require(length == 0, "PNG IEND chunk length is wrong.");
				sawIEND = true;
				break;
			}
			offset = crcOffset + 4;
		}

		require(sawIHDR, "PNG file is missing IHDR.");
		require(sawIEND, "PNG file is missing IEND.");
		require(!png.imageData.empty(), "PNG file is missing IDAT data.");
		return (png);
	}

	void	requirePNGPixel(
		const std::vector<unsigned char>& scanlines,
		std::uint32_t width,
		std::uint32_t x,
		std::uint32_t y,
		unsigned char red,
		unsigned char green,
		unsigned char blue,
		const std::string& message
	)
	{
		const std::size_t rowSize = 1 + (static_cast<std::size_t>(width) * 3);
		const std::size_t rowOffset = static_cast<std::size_t>(y) * rowSize;
		const std::size_t pixelOffset = rowOffset + 1 + (static_cast<std::size_t>(x) * 3);

		require(scanlines.size() >= pixelOffset + 3, "PNG scanlines are too small.");
		require(scanlines[rowOffset] == 0, "PNG scanline filter is not none.");
		require(
			scanlines[pixelOffset] == red
			&& scanlines[pixelOffset + 1] == green
			&& scanlines[pixelOffset + 2] == blue,
			message
		);
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

	void	testToneMappingCompressesHighlights(void)
	{
		Image image(1, 1);
		image.initialize();
		image.setPixel(0, 0, Color(4.0, 2.0, 1.0));

		image.toneMap();
		const Color pixel = image.getPixel(0, 0);

		require(pixel.getRed() > pixel.getGreen(), "Tone mapping did not preserve channel order.");
		require(pixel.getGreen() > pixel.getBlue(), "Tone mapping did not preserve highlight color.");
		require(pixel.getRed() <= 1.0, "Tone mapping did not compress red highlight.");
		require(pixel.getGreen() <= 1.0, "Tone mapping did not compress green highlight.");
		require(pixel.getBlue() <= 1.0, "Tone mapping did not compress blue highlight.");
		require(pixel.getRed() > 0.0, "Tone mapping crushed red highlight.");
	}

	void	testSRGBGammaCorrection(void)
	{
		Image image(1, 1);
		image.initialize();
		image.setPixel(0, 0, Color(0.0031308, 0.25, -1.0));

		image.gammaCorrect();
		const Color pixel = image.getPixel(0, 0);

		requireNear(pixel.getRed(), 0.040449936, "sRGB gamma linear segment is wrong.");
		requireNear(pixel.getGreen(), 0.5370987304831942, "sRGB gamma power segment is wrong.");
		requireNear(pixel.getBlue(), 0.0, "sRGB gamma did not clamp negative values.");
	}

	void	testExposureAndContrast(void)
	{
		Image image(1, 1);
		image.initialize();
		image.setPixel(0, 0, Color(0.25, 0.5, 1.0));

		image.applyExposure(1.0);
		Color pixel = image.getPixel(0, 0);
		requireNear(pixel.getRed(), 0.5, "Exposure did not double red channel.");
		requireNear(pixel.getGreen(), 1.0, "Exposure did not double green channel.");
		requireNear(pixel.getBlue(), 2.0, "Exposure did not double blue channel.");

		image.setPixel(0, 0, Color(0.25, 0.5, 0.75));
		image.applyContrast(2.0);
		pixel = image.getPixel(0, 0);
		requireNear(pixel.getRed(), 0.0, "Contrast lower endpoint is wrong.");
		requireNear(pixel.getGreen(), 0.5, "Contrast pivot is wrong.");
		requireNear(pixel.getBlue(), 1.0, "Contrast upper endpoint is wrong.");
	}

	void	testBloomExtractionPreservesHighlightColor(void)
	{
		Image image(2, 1);
		image.initialize();
		image.setPixel(0, 0, Color(4.0, 1.0, 0.0));
		image.setPixel(1, 0, Color(0.1, 0.1, 0.1));

		auto bloom = image.extractBloom(1.0, 0.0);
		const Color highlight = bloom->getPixel(0, 0);
		const Color dark = bloom->getPixel(1, 0);

		require(highlight.getRed() > highlight.getGreen(), "Bloom extraction did not preserve highlight hue.");
		requireNear(highlight.getBlue(), 0.0, "Bloom extraction added blue to a warm highlight.");
		requireNear(dark.getRed(), 0.0, "Bloom extraction included sub-threshold red.");
		requireNear(dark.getGreen(), 0.0, "Bloom extraction included sub-threshold green.");
		requireNear(dark.getBlue(), 0.0, "Bloom extraction included sub-threshold blue.");
	}

	void	testGaussianBlurSupportsInPlaceSmallImages(void)
	{
		Image image(3, 3);
		image.initialize();
		image.setPixel(1, 1, Color(1.0, 1.0, 1.0));

		Gaussian::blur(image, image, 3, 1.0);
		const Color center = image.getPixel(1, 1);

		require(std::isfinite(center.getRed()), "In-place Gaussian blur produced non-finite red.");
		require(center.getRed() > 0.0, "In-place Gaussian blur lost the center sample.");
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

	void	requireRGBFloats(
		const std::vector<unsigned char>& bytes,
		std::size_t offset,
		float red,
		float green,
		float blue,
		const std::string& message
	)
	{
		requireNear(readF32(bytes, offset), red, message + " red channel is wrong.");
		requireNear(readF32(bytes, offset + 4), green, message + " green channel is wrong.");
		requireNear(readF32(bytes, offset + 8), blue, message + " blue channel is wrong.");
	}

	void	testNonSquareTIFF(void)
	{
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_test_non_square";
		const std::filesystem::path tiffPath = outputPath.string() + ".tiff";

		Image image(3, 2);
		image.initialize();
		image.setPixel(0, 0, Color(1.5, 0.25, 0.0));
		image.setPixel(1, 0, Color(0.0, 2.25, 0.125));
		image.setPixel(2, 0, Color(0.5, 0.0, 4.0));
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
		require(readU16(bytes, bitsPerSample.valueOrOffset) == 32, "TIFF red bit depth is wrong.");
		require(readU16(bytes, bitsPerSample.valueOrOffset + 2) == 32, "TIFF green bit depth is wrong.");
		require(readU16(bytes, bitsPerSample.valueOrOffset + 4) == 32, "TIFF blue bit depth is wrong.");

		const TiffTag sampleFormat = findTiffTag(bytes, ifdOffset, 339);
		require(sampleFormat.type == 3 && sampleFormat.count == 3, "TIFF SampleFormat tag is wrong.");
		require(readU16(bytes, sampleFormat.valueOrOffset) == 3, "TIFF red sample format is wrong.");
		require(readU16(bytes, sampleFormat.valueOrOffset + 2) == 3, "TIFF green sample format is wrong.");
		require(readU16(bytes, sampleFormat.valueOrOffset + 4) == 3, "TIFF blue sample format is wrong.");

		require(findTiffTag(bytes, ifdOffset, 259).valueOrOffset == 1, "TIFF compression is wrong.");
		require(findTiffTag(bytes, ifdOffset, 262).valueOrOffset == 2, "TIFF photometric interpretation is wrong.");
		require(findTiffTag(bytes, ifdOffset, 277).valueOrOffset == 3, "TIFF samples per pixel is wrong.");
		require(findTiffTag(bytes, ifdOffset, 278).valueOrOffset == 2, "TIFF rows per strip is wrong.");
		require(findTiffTag(bytes, ifdOffset, 279).valueOrOffset == 72, "TIFF strip byte count is wrong.");
		require(findTiffTag(bytes, ifdOffset, 284).valueOrOffset == 1, "TIFF planar configuration is wrong.");

		const std::uint32_t pixelDataOffset = findTiffTag(bytes, ifdOffset, 273).valueOrOffset;
		requireRGBFloats(bytes, pixelDataOffset + 0, 1.5f, 0.25f, 0.0f, "TIFF top-left pixel");
		requireRGBFloats(bytes, pixelDataOffset + 12, 0.0f, 2.25f, 0.125f, "TIFF top-middle pixel");
		requireRGBFloats(bytes, pixelDataOffset + 24, 0.5f, 0.0f, 4.0f, "TIFF top-right pixel");
		requireRGBFloats(bytes, pixelDataOffset + 36, 0.0f, 1.0f, 1.0f, "TIFF bottom-left pixel");
		requireRGBFloats(bytes, pixelDataOffset + 48, 1.0f, 0.0f, 1.0f, "TIFF bottom-middle pixel");
		requireRGBFloats(bytes, pixelDataOffset + 60, 1.0f, 1.0f, 0.0f, "TIFF bottom-right pixel");

		std::filesystem::remove(tiffPath);
	}

	void	writeTestPNG(const std::filesystem::path& outputPath)
	{
		Image image(3, 2);

		image.initialize();
		image.setPixel(0, 0, Color(1.0, 0.0, 0.0));
		image.setPixel(1, 0, Color(0.0, 1.0, 0.0));
		image.setPixel(2, 0, Color(0.0, 0.0, 1.0));
		image.setPixel(0, 1, Color(0.0, 1.0, 1.0));
		image.setPixel(1, 1, Color(1.0, 0.0, 1.0));
		image.setPixel(2, 1, Color(1.0, 1.0, 0.0));
		image.saveToPNG(outputPath.string());
	}

	void	requireTestPNG(const std::filesystem::path& pngPath)
	{
		const PNGData png = readPNG(pngPath);
		const std::vector<unsigned char> scanlines = inflatePNGZlib(png.imageData);
		const std::size_t rowSize = 1 + (png.width * 3);

		require(png.width == 3, "PNG width is wrong.");
		require(png.height == 2, "PNG height is wrong.");
		require(png.bitDepth == 8, "PNG bit depth is wrong.");
		require(png.colorType == 2, "PNG color type is wrong.");
		require(scanlines.size() == rowSize * png.height, "PNG scanline byte count is wrong.");
		require((png.imageData[2] & 0x07) == 0x01, "PNG did not use stored deflate.");

		requirePNGPixel(scanlines, png.width, 0, 0, 255, 0, 0, "PNG top-left pixel is wrong.");
		requirePNGPixel(scanlines, png.width, 1, 0, 0, 255, 0, "PNG top-middle pixel is wrong.");
		requirePNGPixel(scanlines, png.width, 2, 0, 0, 0, 255, "PNG top-right pixel is wrong.");
		requirePNGPixel(scanlines, png.width, 0, 1, 0, 255, 255, "PNG bottom-left pixel is wrong.");
		requirePNGPixel(scanlines, png.width, 1, 1, 255, 0, 255, "PNG bottom-middle pixel is wrong.");
		requirePNGPixel(scanlines, png.width, 2, 1, 255, 255, 0, "PNG bottom-right pixel is wrong.");
	}

	void	testNonSquarePNG(void)
	{
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_test_non_square";
		const std::filesystem::path pngPath = outputPath.string() + ".png";

		writeTestPNG(outputPath);
		requireTestPNG(pngPath);

		std::filesystem::remove(pngPath);
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

	void	testSceneFilePNGOutputSettings(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_png_output_test.luz";
		const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "luz_custom_output.png";
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
			"Scene PNG outputfilename setting was not preserved."
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

	void	testSceneFileUsesSceneDefaults(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_defaults_test.luz";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< "resolution=2,2\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());
		require(scene.getAdaptiveSampling(), "Scene file did not keep default adaptive sampling.");
		require(scene.getDenoise(), "Scene file did not keep default denoising.");

		std::filesystem::remove(scenePath);
	}

	void	testSceneFileAdaptiveSettings(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_adaptive_test.luz";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< "adaptive=1\n"
				<< "adaptiveminsamples=32\n"
				<< "adaptivethreshold=0.015\n"
				<< "adaptivecheckinterval=8\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());
		require(scene.getAdaptiveSampling(), "Scene adaptive setting was not applied.");
		require(scene.getAdaptiveMinSamples() == 32, "Scene adaptive minimum samples setting was not applied.");
		requireNear(scene.getAdaptiveThreshold(), 0.015, "Scene adaptive threshold setting was not applied.");
		require(scene.getAdaptiveCheckInterval() == 8, "Scene adaptive check interval setting was not applied.");

		std::filesystem::remove(scenePath);
	}

	void	testSceneFilePostProcessSettings(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_scene_postprocess_test.luz";
		{
			std::ofstream stream(scenePath);
			stream
				<< "[settings]\n"
				<< "gamma=0\n"
				<< "tonemapping=0\n"
				<< "bloom=0\n"
				<< "exposure=1.5\n"
				<< "contrast=1.25\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());
		require(!scene.getGammaCorrected(), "Scene gamma setting was not applied.");
		require(!scene.getToneMapped(), "Scene tonemapping setting was not applied.");
		require(!scene.getBloom(), "Scene bloom setting was not applied.");
		requireNear(scene.getExposure(), 1.5, "Scene exposure setting was not applied.");
		requireNear(scene.getContrast(), 1.25, "Scene contrast setting was not applied.");

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
		requireSceneFileSettingThrows("adaptive=2", "Scene file accepted non-binary adaptive setting.");
		requireSceneFileSettingThrows("adaptiveminsamples=0", "Scene file accepted zero adaptive minimum samples.");
		requireSceneFileSettingThrows("adaptivethreshold=0", "Scene file accepted zero adaptive threshold.");
		requireSceneFileSettingThrows("adaptivecheckinterval=0", "Scene file accepted zero adaptive check interval.");
		requireSceneFileSettingThrows("maxlightbounces=-1", "Scene file accepted negative max light bounces.");
		requireSceneFileSettingThrows("gamma=2", "Scene file accepted non-binary gamma.");
		requireSceneFileSettingThrows("tonemapping=2", "Scene file accepted non-binary tone mapping.");
		requireSceneFileSettingThrows("bloom=2", "Scene file accepted non-binary bloom.");
		requireSceneFileSettingThrows("exposure=nan", "Scene file accepted non-finite exposure.");
		requireSceneFileSettingThrows("contrast=-1", "Scene file accepted negative contrast.");
		requireSceneFileSettingThrows("denoise=2", "Scene file accepted non-binary denoise.");
		requireSceneFileSettingThrows("compression=75", "Scene file accepted removed compression setting.");
		requireSceneFileSettingThrows("outputfilename=render.tif", "Scene file accepted .tif output.");
		requireSceneFileSettingThrows("outputfilename=render.jpg", "Scene file accepted unsupported output extension.");
		requireSceneFileSettingThrows("denoiseoutputfilename=", "Scene file accepted empty denoise output name.");
		requireSceneFileSettingThrows("denoiseoutputfilename=render_denoised", "Scene file accepted denoise output without an extension.");
		requireSceneFileSettingThrows("denoiseoutputfilename=render_denoised.tif", "Scene file accepted .tif denoise output.");
		requireSceneFileSettingThrows("denoiseoutputfilename=render_denoised.jpg", "Scene file accepted unsupported denoise output extension.");
		requireSceneFileSettingThrows("sky=wat", "Scene file accepted unknown sky setting.");
		requireSceneFileSettingThrows("environmentstrength=-1", "Scene file accepted negative environment strength.");
		requireSceneFileSettingThrows("environmentrotation=nan", "Scene file accepted non-finite environment rotation.");
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

	void	writeSinglePixelPPM(const std::filesystem::path& path, int red, int green, int blue, int maxValue = 10)
	{
		std::ofstream textureStream(path);
		textureStream
			<< "P3\n"
			<< "1 1\n"
			<< maxValue << "\n"
			<< red << " " << green << " " << blue << "\n";
	}

	void	testEnvironmentMapLoadsPPM(void)
	{
		const std::filesystem::path environmentPath = std::filesystem::temp_directory_path() / "luz_environment_ppm_test.ppm";
		writeSinglePixelPPM(environmentPath, 1, 2, 3);

		const EnvironmentMap environmentMap = EnvironmentMap::load(environmentPath.string());
		const Color color = environmentMap.sampleDirection(Vector3(1.0, 0.0, 0.0));
		const EnvironmentMap::Sample sample = environmentMap.sample(0.5, Sampler::Sample2D{0.5, 0.5});

		require(environmentMap.getWidth() == 1, "PPM environment width was not loaded.");
		require(environmentMap.getHeight() == 1, "PPM environment height was not loaded.");
		requireNear(color.getRed(), 0.1, "PPM environment red channel was not sampled.");
		requireNear(color.getGreen(), 0.2, "PPM environment green channel was not sampled.");
		requireNear(color.getBlue(), 0.3, "PPM environment blue channel was not sampled.");
		require(sample.valid, "PPM environment importance sample was invalid.");
		require(std::isfinite(sample.pdf) && sample.pdf > 0.0, "PPM environment sample PDF was invalid.");

		std::filesystem::remove(environmentPath);
	}

	void	testEnvironmentMapLoadsHDR(void)
	{
		const std::filesystem::path environmentPath = std::filesystem::temp_directory_path() / "luz_environment_hdr_test.hdr";
		{
			std::ofstream stream(environmentPath, std::ios::binary);
			const unsigned char pixel[4] = {128, 64, 32, 130};

			stream << "#?RADIANCE\n";
			stream << "FORMAT=32-bit_rle_rgbe\n";
			stream << "\n";
			stream << "-Y 1 +X 1\n";
			stream.write(reinterpret_cast<const char*>(pixel), 4);
		}

		const EnvironmentMap environmentMap = EnvironmentMap::load(environmentPath.string());
		const Color color = environmentMap.sampleDirection(Vector3(0.0, 1.0, 0.0));

		requireNear(color.getRed(), 2.0, "HDR environment red channel was not decoded.");
		requireNear(color.getGreen(), 1.0, "HDR environment green channel was not decoded.");
		requireNear(color.getBlue(), 0.5, "HDR environment blue channel was not decoded.");

		std::filesystem::remove(environmentPath);
	}

	void	testEnvironmentMapLoadsRLEHDR(void)
	{
		const std::filesystem::path environmentPath = std::filesystem::temp_directory_path() / "luz_environment_rle_hdr_test.hdr";
		{
			std::ofstream stream(environmentPath, std::ios::binary);
			const unsigned char scanlineHeader[4] = {2, 2, 0, 8};
			const unsigned char redRun[2] = {136, 128};
			const unsigned char greenRun[2] = {136, 64};
			const unsigned char blueRun[2] = {136, 32};
			const unsigned char exponentRun[2] = {136, 130};

			stream << "#?RADIANCE\n";
			stream << "FORMAT=32-bit_rle_rgbe\n";
			stream << "\n";
			stream << "-Y 1 +X 8\n";
			stream.write(reinterpret_cast<const char*>(scanlineHeader), 4);
			stream.write(reinterpret_cast<const char*>(redRun), 2);
			stream.write(reinterpret_cast<const char*>(greenRun), 2);
			stream.write(reinterpret_cast<const char*>(blueRun), 2);
			stream.write(reinterpret_cast<const char*>(exponentRun), 2);
		}

		const EnvironmentMap environmentMap = EnvironmentMap::load(environmentPath.string());
		const Color color = environmentMap.sampleDirection(Vector3(1.0, 0.0, 0.0));

		require(environmentMap.getWidth() == 8, "RLE HDR environment width was not loaded.");
		requireNear(color.getRed(), 2.0, "RLE HDR environment red channel was not decoded.");
		requireNear(color.getGreen(), 1.0, "RLE HDR environment green channel was not decoded.");
		requireNear(color.getBlue(), 0.5, "RLE HDR environment blue channel was not decoded.");

		std::filesystem::remove(environmentPath);
	}

	void	testSceneFileEnvironmentSetting(void)
	{
		const std::filesystem::path directory = std::filesystem::temp_directory_path();
		const std::filesystem::path scenePath = directory / "luz_environment_setting_test.luz";
		const std::filesystem::path environmentPath = directory / "luz_environment_setting_test.ppm";

		writeSinglePixelPPM(environmentPath, 2, 4, 6);
		{
			std::ofstream sceneStream(scenePath);
			sceneStream
				<< "[settings]\n"
				<< "environment=" << environmentPath.filename().string() << ",2.5,90\n\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.getRenderSky() == SKY_ENVIRONMENT, "Environment setting did not enable environment sky.");
		require(scene.hasEnvironmentMap(), "Environment setting did not load an environment map.");
		requireNear(scene.getEnvironmentStrength(), 2.5, "Environment strength was not parsed.");
		requireNear(scene.getEnvironmentRotation(), 90.0, "Environment rotation was not parsed.");

		std::filesystem::remove(scenePath);
		std::filesystem::remove(environmentPath);
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
		const std::filesystem::path directory = std::filesystem::temp_directory_path();
		const std::filesystem::path scenePath = directory / "luz_relative_object_test.luz";
		const std::filesystem::path objectPath = directory / "luz_relative_object_test.obj";
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
				<< "[meshes]\n"
				<< "mesh triangle_mesh {\n"
				<< "file=" << objectPath.filename().string() << "\n"
				<< "}\n\n"
				<< "[scene]\n"
				<< "camera main {\n"
				<< "position=(0,0,5)\n"
				<< "direction=(0,0,-1)\n"
				<< "fov=45\n"
				<< "aperture=0\n"
				<< "focusDistance=5\n"
				<< "}\n"
				<< "object triangle {\n"
				<< "mesh=triangle_mesh\n"
				<< "}\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.hasCamera(), "Relative-object scene did not load a camera.");
		require(scene.getHittables().size() == 1, "Relative-object scene did not load one hittable.");

		std::filesystem::remove(scenePath);
		std::filesystem::remove(objectPath);
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
				<< "directional_light sun {\n"
				<< "direction=(0,-1,0)\n"
				<< "color=(1,0.95,0.8)\n"
				<< "intensity=2\n"
				<< "}\n"
				<< "point_light fill {\n"
				<< "position=(2,2,2)\n"
				<< "radius=0.25\n"
				<< "color=(0.5,0.6,1.0)\n"
				<< "intensity=1.5\n"
				<< "visible=0\n"
				<< "}\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());

		require(scene.hasCamera(), "Named-block scene did not load a camera.");
		requireNear(scene.getActiveCamera().getUpDirection().getY(), 1.0, "Named-block camera up vector was not parsed.");
		require(scene.getHittables().size() == 4, "Named-block scene did not load object and lights.");
		require(scene.getHittables()[0]->getMaterial()->getType() == METAL, "Principled material did not map to metal.");
		require(scene.getHittables()[1]->getMaterial()->getType() == EMISSIVE, "Area light did not create an emissive hittable.");
		require(scene.getHittables()[2]->getMaterial()->getType() == EMISSIVE, "Directional light did not create an emissive hittable.");
		require(scene.getHittables()[3]->getMaterial()->getType() == EMISSIVE, "Point light did not create an emissive hittable.");
		auto pointLight = std::dynamic_pointer_cast<Sphere>(scene.getHittables()[3]);
		require(pointLight != nullptr, "Point light did not create a sphere light.");
		require(!pointLight->isVisible(), "Point light visible flag was not parsed.");
		Ray pointLightRay(Vector3(2.0, 2.0, 3.0), Vector3(0.0, 0.0, -1.0));
		HitRecord pointLightHit;
		require(!pointLight->hit(pointLightRay, pointLightHit, 0.001, 100.0), "Invisible point light was visible to camera rays.");
		HittableLightSample pointLightSample;
		require(pointLight->sampleLight(Vector3(0.0, 0.0, 0.0), pointLightSample), "Invisible point light could not be sampled.");

		scene.updateLights();
		require(scene.getLights().size() == 3, "Named-block lights were not registered as lights.");

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
				<< "emission=(0,0,0)\n"
				<< "emissionStrength=1\n"
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

	void	testSceneFileLoadsTexturedOBJUVs(void)
	{
		const std::filesystem::path directory = std::filesystem::temp_directory_path();
		const std::filesystem::path scenePath = directory / "luz_texture_uv_test.luz";
		const std::filesystem::path objectPath = directory / "luz_texture_uv_test.obj";
		const std::filesystem::path texturePath = directory / "luz_texture_uv_test.ppm";
		{
			std::ofstream textureStream(texturePath);
			textureStream
				<< "P3\n"
				<< "1 1\n"
				<< "255\n"
				<< "255 0 0\n";
		}
		{
			std::ofstream objectStream(objectPath);
			objectStream
				<< "v 0.0 0.0 -1.0\n"
				<< "v 1.0 0.0 -1.0\n"
				<< "v 0.0 1.0 -1.0\n"
				<< "vt 0.0 0.0\n"
				<< "vt 1.0 0.0\n"
				<< "vt 0.0 1.0\n"
				<< "f 1/1 2/2 3/3\n";
		}
		{
			std::ofstream sceneStream(scenePath);
			sceneStream
				<< "[settings]\n"
				<< "resolution=2,2\n\n"
				<< "[materials]\n"
				<< "material textured {\n"
				<< "type=lambertian\n"
				<< "color=(1,1,1)\n"
				<< "texture=" << texturePath.filename().string() << "\n"
				<< "}\n\n"
				<< "[meshes]\n"
				<< "mesh triangle_mesh {\n"
				<< "file=" << objectPath.filename().string() << "\n"
				<< "}\n\n"
				<< "[scene]\n"
				<< "camera=(0,0,2),(0,0,-1),45,0,2\n"
				<< "object triangle {\n"
				<< "mesh=triangle_mesh\n"
				<< "position=(0,0,0)\n"
				<< "rotation=(0,0,0)\n"
				<< "scale=(1,1,1)\n"
				<< "material=textured\n"
				<< "}\n";
		}

		Scene scene;
		SceneFile::read(scene, scenePath.string());
		Ray ray(Vector3(0.25, 0.25, 0.0), Vector3(0.0, 0.0, -1.0));
		HitRecord hitRecord;

		require(scene.getHittables()[0]->hit(ray, hitRecord, 0.001, 100.0), "Textured OBJ triangle was not hit.");
		requireNear(hitRecord.u, 0.25, "OBJ reader did not interpolate texture coordinate U.");
		requireNear(hitRecord.v, 0.25, "OBJ reader did not interpolate texture coordinate V.");
		const Color texturedColor = hitRecord.material->colorAt(hitRecord);
		requireNear(texturedColor.getRed(), 1.0, "Texture red channel was not sampled.");
		requireNear(texturedColor.getGreen(), 0.0, "Texture green channel was not sampled.");
		requireNear(texturedColor.getBlue(), 0.0, "Texture blue channel was not sampled.");

		std::filesystem::remove(scenePath);
		std::filesystem::remove(objectPath);
		std::filesystem::remove(texturePath);
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

	void	testAABBHandlesAxisParallelBoundaryRays(void)
	{
		const AABB boundingBox(Vector3(0.0, 0.0, -2.0), Vector3(1.0, 1.0, -1.0));
		HitRecord hitRecord;
		Ray boundaryRay(Vector3(0.0, 0.5, 0.0), Vector3(0.0, 0.0, -1.0));
		Ray outsideRay(Vector3(2.0, 0.5, 0.0), Vector3(0.0, 0.0, -1.0));
		Ray zeroDirectionRay(Vector3(0.5, 0.5, -1.5), Vector3(0.0, 0.0, 0.0));

		require(boundingBox.hit(boundaryRay, hitRecord, 100.0), "Axis-parallel boundary ray missed the AABB.");
		require(!boundingBox.hit(outsideRay, hitRecord, 100.0), "Axis-parallel outside ray hit the AABB.");
		require(!boundingBox.hit(zeroDirectionRay, hitRecord, 100.0), "Zero-direction ray hit the AABB.");
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
		argv.push_back(const_cast<char*>("luz"));
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
		argv.push_back(const_cast<char*>("luz"));
		for (std::string& argument : arguments)
		{
			argv.push_back(argument.data());
		}

		auto scene = std::make_unique<Scene>();
		FlagsParser(static_cast<int>(argv.size()), argv.data()).parse(*scene);
		return (scene);
	}

	void	testFlagsParseBenchmarkFileOptions(void)
	{
		std::unique_ptr<Scene> bounceScene = parseFlags({"--max-light-bounces", "5"});
		require(bounceScene->getMaxLightBounces() == 5, "--max-light-bounces was not parsed.");

		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_cli_file_benchmark_test.luz";
		std::ofstream sceneFile(scenePath);

		sceneFile
			<< "[settings]\n"
			<< "resolution=3,2\n"
			<< "samples=7\n"
			<< "maxlightbounces=4\n\n";
		sceneFile.close();

		std::unique_ptr<Scene> fileBenchmarkScene = parseFlags({"--file", scenePath.string(), "--benchmark"});
		require(fileBenchmarkScene->getBenchmarkMode(), "--benchmark did not enable benchmark mode for a file scene.");
		require(fileBenchmarkScene->getSampleCount() == 7, "--benchmark replaced the loaded file scene samples.");
		require(fileBenchmarkScene->getMaxLightBounces() == 4, "--benchmark replaced the loaded file scene bounces.");
		require(fileBenchmarkScene->getImage()->getWidth() == 3, "--benchmark replaced the loaded file scene width.");
		require(fileBenchmarkScene->getImage()->getHeight() == 2, "--benchmark replaced the loaded file scene height.");

		std::filesystem::remove(scenePath);
	}

	void	testFlagsParsePositionalSceneFile(void)
	{
		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_cli_positional_file_test.luz";
		std::ofstream sceneFile(scenePath);

		sceneFile
			<< "[settings]\n"
			<< "resolution=3,2\n"
			<< "samples=7\n"
			<< "maxlightbounces=4\n\n";
		sceneFile.close();

		std::unique_ptr<Scene> scene = parseFlags({scenePath.string()});
		require(scene->getIsFromFile(), "Positional scene path was not loaded.");
		require(scene->getSampleCount() == 7, "Positional scene path did not load scene samples.");
		require(scene->getMaxLightBounces() == 4, "Positional scene path did not load scene bounces.");
		require(scene->getImage()->getWidth() == 3, "Positional scene path did not load scene width.");
		require(scene->getImage()->getHeight() == 2, "Positional scene path did not load scene height.");

		std::unique_ptr<Scene> overrideScene = parseFlags({
			scenePath.string(),
			"--samples", "9",
			"--resolution", "4x5"
		});
		require(overrideScene->getSampleCount() == 9, "CLI samples did not override positional scene file.");
		require(overrideScene->getImage()->getWidth() == 4, "CLI width did not override positional scene file.");
		require(overrideScene->getImage()->getHeight() == 5, "CLI height did not override positional scene file.");

		std::filesystem::remove(scenePath);
	}

	void	testFlagsParsePostProcessOptions(void)
	{
		std::unique_ptr<Scene> scene = parseFlags({
			"--gamma", "false",
			"--tonemapping", "false",
			"--bloom", "false",
			"--exposure", "1.25",
			"--contrast", "1.5"
		});

		require(!scene->getGammaCorrected(), "--gamma false was not parsed.");
		require(!scene->getToneMapped(), "--tonemapping false was not parsed.");
		require(!scene->getBloom(), "--bloom false was not parsed.");
		requireNear(scene->getExposure(), 1.25, "--exposure was not parsed.");
		requireNear(scene->getContrast(), 1.5, "--contrast was not parsed.");
	}

	void	testFlagsParseDenoiseOptions(void)
	{
		std::unique_ptr<Scene> enabledScene = parseFlags({"--denoise"});
		require(enabledScene->getDenoise(), "Bare --denoise did not enable denoising.");

		std::unique_ptr<Scene> falseScene = parseFlags({"--denoise", "false"});
		require(!falseScene->getDenoise(), "--denoise false did not disable denoising.");

		std::unique_ptr<Scene> disabledScene = parseFlags({"--denoise", "--no-denoise"});
		require(!disabledScene->getDenoise(), "--no-denoise did not override --denoise.");

		std::unique_ptr<Scene> reenabledScene = parseFlags({"--no-denoise", "--denoise"});
		require(reenabledScene->getDenoise(), "Later --denoise did not override --no-denoise.");

		std::unique_ptr<Scene> outputScene = parseFlags({"--denoise-output", "custom_denoised.tiff"});
		require(
			outputScene->getDenoiseOutputFileName() == "custom_denoised.tiff",
			"--denoise-output was not parsed."
		);
		std::unique_ptr<Scene> rawOutputScene = parseFlags({"--output", "custom_render.tiff"});
		require(
			rawOutputScene->getDefaultRenderOutputFileName() == "custom_render.tiff",
			"--output was not parsed."
		);
		std::unique_ptr<Scene> shortOutputScene = parseFlags({"-o", "short_render.bmp"});
		require(
			shortOutputScene->getDefaultRenderOutputFileName() == "short_render.bmp",
			"-o was not parsed."
		);

		const std::filesystem::path scenePath = std::filesystem::temp_directory_path() / "luz_cli_denoise_override_test.luz";
		std::ofstream sceneFile(scenePath);

		sceneFile
			<< "[settings]\n"
			<< "resolution=1,1\n"
			<< "denoise=0\n\n";
		sceneFile.close();

		std::unique_ptr<Scene> fileOverrideScene = parseFlags({"--file", scenePath.string(), "--denoise"});
		require(fileOverrideScene->getDenoise(), "--denoise did not override scene file denoise setting.");
	}

	void	testFlagsParsePNGOutput(void)
	{
		std::unique_ptr<Scene> outputScene = parseFlags({"--output", "custom_render.png"});
		require(
			outputScene->getDefaultRenderOutputFileName() == "custom_render.png",
			"PNG --output was not parsed."
		);
	}

	void	testFlagsParseAdaptiveOptions(void)
	{
		std::unique_ptr<Scene> enabledScene = parseFlags({"--adaptive"});
		require(enabledScene->getAdaptiveSampling(), "Bare --adaptive did not enable adaptive sampling.");

		std::unique_ptr<Scene> falseScene = parseFlags({"--adaptive", "false"});
		require(!falseScene->getAdaptiveSampling(), "--adaptive false did not disable adaptive sampling.");

		std::unique_ptr<Scene> disabledScene = parseFlags({"--adaptive", "--no-adaptive"});
		require(!disabledScene->getAdaptiveSampling(), "--no-adaptive did not override --adaptive.");

		std::unique_ptr<Scene> tunedScene = parseFlags({
			"--adaptive",
			"--adaptive-min-samples", "24",
			"--adaptive-threshold", "0.03",
			"--adaptive-check-interval", "6"
		});
		require(tunedScene->getAdaptiveSampling(), "Adaptive sampling was not enabled.");
		require(tunedScene->getAdaptiveMinSamples() == 24, "--adaptive-min-samples was not parsed.");
		requireNear(tunedScene->getAdaptiveThreshold(), 0.03, "--adaptive-threshold was not parsed.");
		require(tunedScene->getAdaptiveCheckInterval() == 6, "--adaptive-check-interval was not parsed.");
	}

	void	testFlagsRejectInvalidValues(void)
	{
		requireFlagParseThrows({"--samples", "0"}, "CLI accepted zero samples.");
		requireFlagParseThrows({"--adaptive", "maybe"}, "CLI accepted invalid adaptive value.");
		requireFlagParseThrows({"--adaptive-min-samples", "0"}, "CLI accepted zero adaptive minimum samples.");
		requireFlagParseThrows({"--adaptive-threshold", "0"}, "CLI accepted zero adaptive threshold.");
		requireFlagParseThrows({"--adaptive-check-interval", "0"}, "CLI accepted zero adaptive check interval.");
		requireFlagParseThrows({"--maxLightBounces", "-1"}, "CLI accepted negative max light bounces.");
		requireFlagParseThrows({"--resolution", "-1x100"}, "CLI accepted negative width.");
		requireFlagParseThrows({"--threads", "0"}, "CLI accepted zero threads.");
		requireFlagParseThrows({"--exposure"}, "CLI accepted missing exposure.");
		requireFlagParseThrows({"--exposure", "nan"}, "CLI accepted non-finite exposure.");
		requireFlagParseThrows({"--contrast"}, "CLI accepted missing contrast.");
		requireFlagParseThrows({"--contrast", "-1"}, "CLI accepted negative contrast.");
		requireFlagParseThrows({"--denoise", "maybe"}, "CLI accepted invalid denoise value.");
		requireFlagParseThrows({"--compression", "75"}, "CLI accepted removed compression flag.");
		requireFlagParseThrows({"--compression=75"}, "CLI accepted removed compression assignment.");
		requireFlagParseThrows({"--output-file", "render.tiff"}, "CLI accepted removed output-file flag.");
		requireFlagParseThrows({"--output-file=render.tiff"}, "CLI accepted removed output-file assignment.");
		requireFlagParseThrows({"--output"}, "CLI accepted missing output path.");
		requireFlagParseThrows({"--output", "tiff"}, "CLI accepted output format instead of output path.");
		requireFlagParseThrows({"--output", "render"}, "CLI accepted output path without an extension.");
		requireFlagParseThrows({"--output", "render.tif"}, "CLI accepted .tif output extension.");
		requireFlagParseThrows({"--output", "render.jpg"}, "CLI accepted unsupported output extension.");
		requireFlagParseThrows({"--denoise-output"}, "CLI accepted missing denoise output path.");
		requireFlagParseThrows({"--denoise-output", "render"}, "CLI accepted denoise output path without an extension.");
		requireFlagParseThrows({"--denoise-output", "render_denoised.tif"}, "CLI accepted .tif denoise output extension.");
		requireFlagParseThrows({"--denoise-output", "render.jpg"}, "CLI accepted unsupported denoise output extension.");
	}

	void	testSettersRejectInvalidValues(void)
	{
		Scene scene;
		Image image;

		requireThrows([&]() { scene.setSampleCount(0); }, "Scene accepted zero samples.");
		requireThrows([&]() { scene.setAdaptiveMinSamples(0); }, "Scene accepted zero adaptive minimum samples.");
		requireThrows([&]() { scene.setAdaptiveThreshold(0.0); }, "Scene accepted zero adaptive threshold.");
		requireThrows([&]() { scene.setAdaptiveCheckInterval(0); }, "Scene accepted zero adaptive check interval.");
		requireThrows([&]() { scene.setMaxLightBounces(-1); }, "Scene accepted negative max light bounces.");
		requireThrows([&]() { scene.setRenderingThreads(0); }, "Scene accepted zero rendering threads.");
		requireThrows([&]() { scene.setExposure(std::numeric_limits<double>::quiet_NaN()); }, "Scene accepted non-finite exposure.");
		requireThrows([&]() { scene.setContrast(-1.0); }, "Scene accepted negative contrast.");
		requireThrows([&]() { scene.setContrast(std::numeric_limits<double>::quiet_NaN()); }, "Scene accepted non-finite contrast.");
		requireThrows([&]() { scene.setEnvironmentStrength(-1.0); }, "Scene accepted negative environment strength.");
		requireThrows([&]() { scene.setEnvironmentStrength(std::numeric_limits<double>::quiet_NaN()); }, "Scene accepted non-finite environment strength.");
		requireThrows([&]() { scene.setEnvironmentRotation(std::numeric_limits<double>::quiet_NaN()); }, "Scene accepted non-finite environment rotation.");
		requireThrows([&]() { scene.setDefaultRenderOutputFileName("render"); }, "Scene accepted output file without an extension.");
		requireThrows([&]() { scene.setDefaultRenderOutputFileName("render.tif"); }, "Scene accepted .tif output extension.");
		requireThrows([&]() { scene.setDefaultRenderOutputFileName("render.jpg"); }, "Scene accepted unsupported output extension.");
		requireThrows([&]() { scene.setDenoiseOutputFileName("render_denoised"); }, "Scene accepted denoise output without an extension.");
		requireThrows([&]() { scene.setDenoiseOutputFileName("render_denoised.tif"); }, "Scene accepted .tif denoise output extension.");
		requireThrows([&]() { scene.setDenoiseOutputFileName("render_denoised.jpg"); }, "Scene accepted unsupported denoise output extension.");
		requireThrows([&]() { image.setWidth(0); }, "Image accepted zero width.");
		requireThrows([&]() { image.setHeight(0); }, "Image accepted zero height.");
		requireThrows([&]() { Atmosphere().setSamples(0); }, "Atmosphere accepted zero samples.");
		requireThrows([&]() { Atmosphere().setLightSamples(0); }, "Atmosphere accepted zero light samples.");
	}

	void	testSceneDefaultsEnableAdaptiveAndDenoise(void)
	{
		Scene scene;

		require(scene.getAdaptiveSampling(), "Adaptive sampling is not enabled by default.");
		require(scene.getDenoise(), "Denoising is not enabled by default.");
	}

	void	testTinyRender(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(4);
		scene.getImage()->setHeight(3);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setAdaptiveSampling(false);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setDenoise(false);
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

	void	testEnvironmentBackgroundRender(void)
	{
		setRandomSeed(42);

		const std::filesystem::path environmentPath = std::filesystem::temp_directory_path() / "luz_environment_render_test.ppm";
		writeSinglePixelPPM(environmentPath, 2, 4, 6);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setMaxLightBounces(0);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setRenderSky(SKY_ENVIRONMENT);
		scene.setEnvironmentMap(std::make_shared<EnvironmentMap>(EnvironmentMap::load(environmentPath.string())));
		scene.setEnvironmentStrength(1.5);
		scene.setRenderingThreads(1);
		scene.addCamera(Camera(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0), 45, 0.0, 1.0));

		require(Renderer::render(scene), "Environment background render failed.");
		for (std::size_t y = 0; y < scene.getImage()->getHeight(); y++)
		{
			for (std::size_t x = 0; x < scene.getImage()->getWidth(); x++)
			{
				const Color pixel = scene.getImage()->getPixel(x, y);
				requireNear(pixel.getRed(), 0.3, "Environment render red channel was wrong.");
				requireNear(pixel.getGreen(), 0.6, "Environment render green channel was wrong.");
				requireNear(pixel.getBlue(), 0.9, "Environment render blue channel was wrong.");
			}
		}

		std::filesystem::remove(environmentPath);
	}

	void	testTinyAdaptiveRender(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(4);
		scene.setAdaptiveSampling(true);
		scene.setAdaptiveMinSamples(2);
		scene.setAdaptiveThreshold(0.5);
		scene.setAdaptiveCheckInterval(1);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setDenoise(false);
		scene.setRenderSky(SKY_NONE);
		scene.setBackgroundColor(Color(0.1, 0.2, 0.3));
		scene.setRenderingThreads(1);
		scene.addCamera(Camera(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0), 45, 0.0, 1.0));

		require(Renderer::render(scene), "Tiny adaptive render failed.");
		for (std::size_t y = 0; y < scene.getImage()->getHeight(); y++)
		{
			for (std::size_t x = 0; x < scene.getImage()->getWidth(); x++)
			{
				const Color pixel = scene.getImage()->getPixel(x, y);
				require(std::isfinite(pixel.getRed()), "Tiny adaptive render produced non-finite red value.");
				requireNear(pixel.getRed(), 0.1, "Tiny adaptive render changed stable background red.");
			}
		}
	}

	void	testTinyAdaptiveDenoisedRender(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(4);
		scene.setAdaptiveSampling(true);
		scene.setAdaptiveMinSamples(2);
		scene.setAdaptiveThreshold(0.5);
		scene.setAdaptiveCheckInterval(1);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setDenoise(true);
		scene.setRenderSky(SKY_NONE);
		scene.setBackgroundColor(Color(0.1, 0.2, 0.3));
		scene.setRenderingThreads(1);
		scene.addCamera(Camera(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0), 45, 0.0, 1.0));

		require(Renderer::render(scene), "Tiny adaptive denoised render failed.");
		require(scene.hasDenoisedImage(), "Adaptive denoised render did not create a companion image.");
		const Color denoisedPixel = scene.getDenoisedImage()->getPixel(0, 0);
		require(std::isfinite(denoisedPixel.getRed()), "Adaptive denoised companion red value is non-finite.");
	}

	void	testTinyDenoisedRenderProducesCompanionImage(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setAdaptiveSampling(false);
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
		const Color denoisedPixel = scene.getDenoisedImage()->getPixel(0, 0);
		require(std::isfinite(denoisedPixel.getRed()), "Denoised companion red value is non-finite.");
		require(std::abs(denoisedPixel.getRed() - 0.1) < 0.02, "Denoised companion image value drifted too far.");
	}

	void	testZeroFocusDistanceRender(void)
	{
		setRandomSeed(42);

		Scene scene;
		scene.getImage()->setWidth(2);
		scene.getImage()->setHeight(2);
		scene.getImage()->initialize();
		scene.setSampleCount(1);
		scene.setAdaptiveSampling(false);
		scene.setMaxLightBounces(1);
		scene.setGammaCorrected(false);
		scene.setToneMapped(false);
		scene.setBloom(false);
		scene.setDenoise(false);
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
		testToneMappingCompressesHighlights();
		testSRGBGammaCorrection();
		testExposureAndContrast();
		testBloomExtractionPreservesHighlightColor();
		testGaussianBlurSupportsInPlaceSmallImages();
		testTerminalFilePath();
		testNonSquareBMP();
		testNonSquareTIFF();
		testNonSquarePNG();
		testSceneFileOutputName();
		testSceneFileTiffOutputName();
		testSceneFilePNGOutputSettings();
		testSceneFileDenoiseOutputName();
		testSceneFileUsesSceneDefaults();
		testSceneFileAdaptiveSettings();
		testSceneFilePostProcessSettings();
		testSceneFileRejectsInvalidSettings();
		testSceneFileBackgroundSetting();
		testEnvironmentMapLoadsPPM();
		testEnvironmentMapLoadsHDR();
		testEnvironmentMapLoadsRLEHDR();
		testSceneFileEnvironmentSetting();
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
		testSceneFileLoadsTexturedOBJUVs();
		testMeshPDFAndRandomSampling();
		testAABBDefaultBounds();
		testAABBHandlesAxisParallelBoundaryRays();
		testRectangleBoundingBoxes();
		testRectangleRandomSamplesSupportedAxes();
		testCubeBoundingBoxAndSetters();
		testHittablePDFAveragesMultipleLights();
		testFlagsParsePostProcessOptions();
		testFlagsParseDenoiseOptions();
		testFlagsParsePNGOutput();
		testFlagsParseBenchmarkFileOptions();
		testFlagsParsePositionalSceneFile();
		testFlagsParseAdaptiveOptions();
		testFlagsRejectInvalidValues();
		testSettersRejectInvalidValues();
		testSceneDefaultsEnableAdaptiveAndDenoise();
		testTinyRender();
		testTinyAdaptiveRender();
		testTinyAdaptiveDenoisedRender();
		testTinyDenoisedRenderProducesCompanionImage();
		testZeroFocusDistanceRender();
		testEnvironmentBackgroundRender();
	}
	catch (const std::exception& exception)
	{
		std::cerr << "Test failed: " << exception.what() << std::endl;
		return (1);
	}

	std::cout << "All tests passed." << std::endl;
	return (0);
}
