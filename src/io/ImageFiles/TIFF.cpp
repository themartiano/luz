#include "ImageFiles/TIFF.hpp"
#include "ANSIColors.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <limits>
#include <stdexcept>
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

	constexpr std::uint16_t	TIFF_VERSION = 42;
	constexpr std::uint16_t	TYPE_SHORT = 3;
	constexpr std::uint16_t	TYPE_LONG = 4;
	constexpr std::uint16_t	TAG_COUNT = 10;
	constexpr std::uint32_t	IFD_OFFSET = 8;
	constexpr std::uint32_t	IFD_BYTE_COUNT = sizeof(std::uint16_t) + (TAG_COUNT * 12) + sizeof(std::uint32_t);
	constexpr std::uint32_t	BITS_PER_SAMPLE_OFFSET = IFD_OFFSET + IFD_BYTE_COUNT;
	constexpr std::uint32_t	PIXEL_DATA_OFFSET = BITS_PER_SAMPLE_OFFSET + (3 * sizeof(std::uint16_t));

	void	writeU16(std::ofstream& stream, std::uint16_t value)
	{
		const unsigned char bytes[2] = {
			static_cast<unsigned char>(value & 0xff),
			static_cast<unsigned char>((value >> 8) & 0xff)
		};

		stream.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
	}

	void	writeU32(std::ofstream& stream, std::uint32_t value)
	{
		const unsigned char bytes[4] = {
			static_cast<unsigned char>(value & 0xff),
			static_cast<unsigned char>((value >> 8) & 0xff),
			static_cast<unsigned char>((value >> 16) & 0xff),
			static_cast<unsigned char>((value >> 24) & 0xff)
		};

		stream.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
	}

	void	writeTag(std::ofstream& stream, const TiffTag& tag)
	{
		writeU16(stream, tag.id);
		writeU16(stream, tag.type);
		writeU32(stream, tag.count);
		writeU32(stream, tag.valueOrOffset);
	}

	std::uint32_t	checkedU32(std::size_t value, const std::string& name)
	{
		if (value > std::numeric_limits<std::uint32_t>::max())
		{
			throw std::runtime_error("TIFF " + name + " is too large.");
		}

		return (static_cast<std::uint32_t>(value));
	}

	unsigned char	colorByte(double value)
	{
		if (!std::isfinite(value))
		{
			value = 0.0;
		}
		Utilities::setDoubleRange(value, 0.0, 1.0);
		return (static_cast<unsigned char>((value * 255.0) + 0.5));
	}

	std::vector<unsigned char>	buildPixelData(const std::unique_ptr<Image>& image)
	{
		std::vector<unsigned char> pixels;

		pixels.reserve(image->getWidth() * image->getHeight() * 3);
		for (std::size_t y = 0; y < image->getHeight(); y++)
		{
			for (std::size_t x = 0; x < image->getWidth(); x++)
			{
				const Color pixel = image->getPixel(x, y);

				pixels.push_back(colorByte(pixel.getRed()));
				pixels.push_back(colorByte(pixel.getGreen()));
				pixels.push_back(colorByte(pixel.getBlue()));
			}
		}

		return (pixels);
	}
}

TIFF::TIFF(void)
{
	this->_fileName = D_RENDER_FILE_NAME + ".tiff";
}

TIFF::TIFF(std::string fileName)
{
	this->_fileName = fileName;
}

// Writes a .tiff image file using the information present on 'scene'
void	TIFF::writeFile(const std::unique_ptr<Image>& image, bool insideDir, std::string dirName)
{
	std::string filePath = "";
	if (insideDir == true)
	{
		std::filesystem::create_directories(dirName);
		filePath = dirName + "/";
	}
	filePath += this->_fileName;

	if (!Utilities::stringEndsWith(filePath, ".tiff") && !Utilities::stringEndsWith(filePath, ".tif"))
	{
		filePath += ".tiff";
	}

	if (image->getWidth() == 0 || image->getHeight() == 0)
	{
		throw std::runtime_error("Cannot write an empty TIFF image.");
	}

	const std::uint32_t width = checkedU32(image->getWidth(), "width");
	const std::uint32_t height = checkedU32(image->getHeight(), "height");
	const std::vector<unsigned char> pixelData = buildPixelData(image);
	const std::uint32_t stripByteCount = checkedU32(pixelData.size(), "pixel data");
	const TiffTag tags[TAG_COUNT] = {
		{256, TYPE_LONG, 1, width},
		{257, TYPE_LONG, 1, height},
		{258, TYPE_SHORT, 3, BITS_PER_SAMPLE_OFFSET},
		{259, TYPE_SHORT, 1, 1},
		{262, TYPE_SHORT, 1, 2},
		{273, TYPE_LONG, 1, PIXEL_DATA_OFFSET},
		{277, TYPE_SHORT, 1, 3},
		{278, TYPE_LONG, 1, height},
		{279, TYPE_LONG, 1, stripByteCount},
		{284, TYPE_SHORT, 1, 1}
	};

	std::cout << CLR_YELLOW << "Writing render to " << CLR_BLUE_BRIGHT << Utilities::terminalFilePath(filePath) << "\n" << CLR_RESET;

	std::ofstream stream(filePath, std::ios::binary);
	if (!stream)
	{
		throw std::runtime_error("Could not open TIFF output file: " + filePath);
	}

	stream.write("II", 2);
	writeU16(stream, TIFF_VERSION);
	writeU32(stream, IFD_OFFSET);
	writeU16(stream, TAG_COUNT);
	for (const TiffTag& tag : tags)
	{
		writeTag(stream, tag);
	}
	writeU32(stream, 0);
	writeU16(stream, 8);
	writeU16(stream, 8);
	writeU16(stream, 8);
	stream.write(reinterpret_cast<const char*>(pixelData.data()), pixelData.size());

	if (!stream)
	{
		throw std::runtime_error("Could not write TIFF output file: " + filePath);
	}

	std::cout << CLR_GREEN_BRIGHT << "File ready.\n\n" << CLR_RESET;
}

// TIFF::writeFile overload
void	TIFF::writeFile(const std::unique_ptr<Image>& image)
{
	writeFile(image, false, "");
}
