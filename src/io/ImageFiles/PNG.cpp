#include "ImageFiles/PNG.hpp"
#include "ANSIColors.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
	constexpr unsigned char PNG_SIGNATURE[8] = {137, 80, 78, 71, 13, 10, 26, 10};
	constexpr std::size_t MAX_STORED_BLOCK_SIZE = 65535;

	std::uint32_t	checkedU32(std::size_t value, const std::string& name)
	{
		if (value > std::numeric_limits<std::uint32_t>::max())
		{
			throw std::runtime_error("PNG " + name + " is too large.");
		}

		return (static_cast<std::uint32_t>(value));
	}

	void	appendU32BE(std::vector<unsigned char>& bytes, std::uint32_t value)
	{
		bytes.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
		bytes.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
		bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
		bytes.push_back(static_cast<unsigned char>(value & 0xff));
	}

	void	writeU32BE(std::ofstream& stream, std::uint32_t value)
	{
		const unsigned char bytes[4] = {
			static_cast<unsigned char>((value >> 24) & 0xff),
			static_cast<unsigned char>((value >> 16) & 0xff),
			static_cast<unsigned char>((value >> 8) & 0xff),
			static_cast<unsigned char>(value & 0xff)
		};

		stream.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
	}

	std::uint32_t	crc32(const unsigned char* data, std::size_t size)
	{
		std::uint32_t crc = 0xffffffff;

		for (std::size_t i = 0; i < size; i++)
		{
			crc ^= data[i];
			for (int bit = 0; bit < 8; bit++)
			{
				if ((crc & 1) != 0)
				{
					crc = (crc >> 1) ^ 0xedb88320;
				}
				else
				{
					crc >>= 1;
				}
			}
		}

		return (crc ^ 0xffffffff);
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

	unsigned char	colorByte(double value)
	{
		if (!std::isfinite(value))
		{
			value = 0.0;
		}
		Utilities::setDoubleRange(value, 0.0, 1.0);
		return (static_cast<unsigned char>((value * 255.0) + 0.5));
	}

	std::vector<unsigned char>	buildScanlines(const std::unique_ptr<Image>& image)
	{
		std::vector<unsigned char> scanlines;
		const std::size_t rowSize = 1 + (image->getWidth() * 3);

		scanlines.reserve(rowSize * image->getHeight());
		for (std::size_t y = 0; y < image->getHeight(); y++)
		{
			scanlines.push_back(0);
			for (std::size_t x = 0; x < image->getWidth(); x++)
			{
				const Color pixel = image->getPixel(x, y);

				scanlines.push_back(colorByte(pixel.getRed()));
				scanlines.push_back(colorByte(pixel.getGreen()));
				scanlines.push_back(colorByte(pixel.getBlue()));
			}
		}

		return (scanlines);
	}

	std::vector<unsigned char>	zlibStored(const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> output;
		std::size_t offset = 0;

		output.push_back(0x78);
		output.push_back(0x01);
		do
		{
			const std::size_t remaining = data.size() - offset;
			const std::uint16_t length = static_cast<std::uint16_t>(std::min(remaining, MAX_STORED_BLOCK_SIZE));
			const bool finalBlock = offset + length == data.size();
			const std::uint16_t inverseLength = static_cast<std::uint16_t>(~length);

			output.push_back(finalBlock ? 0x01 : 0x00);
			output.push_back(static_cast<unsigned char>(length & 0xff));
			output.push_back(static_cast<unsigned char>((length >> 8) & 0xff));
			output.push_back(static_cast<unsigned char>(inverseLength & 0xff));
			output.push_back(static_cast<unsigned char>((inverseLength >> 8) & 0xff));
			output.insert(output.end(), data.begin() + offset, data.begin() + offset + length);
			offset += length;
		}
		while (offset < data.size());
		appendU32BE(output, adler32(data));

		return (output);
	}

	void	writeChunk(std::ofstream& stream, const char type[4], const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> crcData;

		writeU32BE(stream, checkedU32(data.size(), "chunk"));
		stream.write(type, 4);
		if (!data.empty())
		{
			stream.write(reinterpret_cast<const char*>(data.data()), data.size());
		}
		crcData.insert(crcData.end(), type, type + 4);
		crcData.insert(crcData.end(), data.begin(), data.end());
		writeU32BE(stream, crc32(crcData.data(), crcData.size()));
	}
}

PNG::PNG(void)
{
	this->_fileName = D_RENDER_FILE_NAME + ".png";
}

PNG::PNG(std::string fileName)
{
	this->_fileName = fileName;
}

void	PNG::writeFile(const std::unique_ptr<Image>& image, bool insideDir, std::string dirName)
{
	std::string filePath = "";
	if (insideDir == true)
	{
		std::filesystem::create_directories(dirName);
		filePath = dirName + "/";
	}
	filePath += this->_fileName;

	std::string lowerFilePath = filePath;
	Utilities::toLower(lowerFilePath);
	if (!Utilities::stringEndsWith(lowerFilePath, ".png"))
	{
		filePath += ".png";
	}

	if (image->getWidth() == 0 || image->getHeight() == 0)
	{
		throw std::runtime_error("Cannot write an empty PNG image.");
	}

	const std::vector<unsigned char> scanlines = buildScanlines(image);
	const std::vector<unsigned char> imageData = zlibStored(scanlines);
	std::vector<unsigned char> ihdr;

	appendU32BE(ihdr, checkedU32(image->getWidth(), "width"));
	appendU32BE(ihdr, checkedU32(image->getHeight(), "height"));
	ihdr.push_back(8);
	ihdr.push_back(2);
	ihdr.push_back(0);
	ihdr.push_back(0);
	ihdr.push_back(0);

	std::cout << CLR_YELLOW << "Writing render to " << CLR_BLUE_BRIGHT << Utilities::terminalFilePath(filePath) << "\n" << CLR_RESET;

	std::ofstream stream(filePath, std::ios::binary);
	if (!stream)
	{
		throw std::runtime_error("Could not open PNG output file: " + filePath);
	}

	stream.write(reinterpret_cast<const char*>(PNG_SIGNATURE), sizeof(PNG_SIGNATURE));
	writeChunk(stream, "IHDR", ihdr);
	writeChunk(stream, "IDAT", imageData);
	writeChunk(stream, "IEND", {});

	if (!stream)
	{
		throw std::runtime_error("Could not write PNG output file: " + filePath);
	}

	std::cout << CLR_GREEN_BRIGHT << "File ready.\n\n" << CLR_RESET;
}

void	PNG::writeFile(const std::unique_ptr<Image>& image)
{
	writeFile(image, false, "");
}
