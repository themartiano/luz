#include "ImageFiles/TIFF.hpp"
#include "ANSIColors.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <iostream>
#include <filesystem>

#define PIXEL_BYTES (8 * 3)
#define RESOLUTION 256
#define TILE_SIZE 16 // multiple of 16
#define TILE_COUNT ((RESOLUTION / TILE_SIZE) * (RESOLUTION / TILE_SIZE))

TIFF::TIFF(void)
{
	this->_fileName = D_RENDER_FILE_NAME + ".tiff";
}

TIFF::TIFF(std::string fileName)
{
	this->_fileName = fileName;
}

// Writes a .tiff image file using the information present on 'scene'
void	TIFF::writeFile(const Image& image, bool insideDir, std::string dirName)
{
	std::string filePath = "";
	if (insideDir == true)
	{
		std::filesystem::create_directory(dirName);
		filePath = dirName + "/";
	}
	filePath += this->_fileName;

	if (!Utilities::stringEndsWith(filePath, ".tiff"))
	{
		filePath += ".tiff";
	}

	std::cout << CLR_YELLOW << "Writing render to " << CLR_BLUE_BRIGHT << filePath << CLR_YELLOW << "...\n" << CLR_RESET;

	FILE* imageFile = fopen(filePath.c_str(), "wb");
	tiffIFD ifd = _generateIFD();

	// Write header
	tiffHeader header;
	header.IFDOffset = sizeof(tiffHeader) + (sizeof(short) * 3) + (TILE_COUNT * sizeof(int) * 2);
	fwrite(&header, sizeof(tiffHeader), 1, imageFile);

	// Write bits per sample
	short	bits = 64;
	for (int i = 0; i < 3; i++)
	{
		fwrite(&bits, sizeof(short), 1, imageFile);
	}

	// Write tile offsets
	int	baseOffset = sizeof(tiffHeader) + (sizeof(short) * 3) + (TILE_COUNT * sizeof(int) * 2) + (sizeof(short) + (sizeof(tiffTag) * ifd.tagCount) + sizeof(int));
	for (int i = 0; i < TILE_COUNT; i++)
	{
		int offset = baseOffset + (i * TILE_SIZE * TILE_SIZE * PIXEL_BYTES);
		fwrite(&offset, sizeof(int), 1, imageFile);
	}

	// Write tile byte count
	int	tileByteCount = TILE_SIZE * TILE_SIZE * PIXEL_BYTES;
	for (int i = 0; i < TILE_COUNT; i++)
	{
		fwrite(&tileByteCount, sizeof(int), 1, imageFile);
	}

	// Write IFD
	fwrite(&ifd.tagCount, sizeof(short), 1, imageFile);
	fwrite(ifd.tagList, sizeof(tiffTag), ifd.tagCount, imageFile);
	fwrite(&ifd.nextIFDOffset, sizeof(int), 1, imageFile);
	delete[] ifd.tagList;

	std::vector<double> pixelArray;
	arrayColorToDouble(image, pixelArray);

	// Write bitmap image data
	for (unsigned int row = 0; row < RESOLUTION / TILE_SIZE; row++)
	{
		unsigned int rowPos = (row * TILE_SIZE * image.getWidth() * 3);
		for (int column = 0; column < RESOLUTION / TILE_SIZE; column++)
		{
			unsigned int columnPos = (column * TILE_SIZE * 3);
			for (unsigned int y = 0; y < TILE_SIZE; y++)
			{
				unsigned int tilePos = (y * image.getWidth() * 3);

				fwrite(pixelArray.data() + rowPos + columnPos + tilePos, PIXEL_BYTES, TILE_SIZE, imageFile);
			}
		}
	}

	fclose(imageFile);
	std::cout << CLR_GREEN_BRIGHT << "File ready.\n\n" << CLR_RESET;
}

// TIFF::writeFile overload
void	TIFF::writeFile(const Image& image)
{
	writeFile(image, false, "");
}

TIFF::tiffIFD	TIFF::_generateIFD(void)
{
	tiffIFD ifd;

	ifd.tagCount = 15;

	tiffTag*	tags = new tiffTag[ifd.tagCount];

	int tag = 0;

	tags[tag].tagId = 254; // NewSubfileType
	tags[tag].dataType = 0x04;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 0;

	tags[tag].tagId = 256; // ImageWidth
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = RESOLUTION;

	tags[tag].tagId = 257; // ImageHeight
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = RESOLUTION;

	tags[tag].tagId = 258; // BitsPerSample
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x03;
	tags[tag++].dataOffset = 0x08;

	tags[tag].tagId = 259; // Compression (none)
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 1;

	tags[tag].tagId = 262; // PhotometricInterpretation (Color Scheme) (RGB)
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 2;

	tags[tag].tagId = 266; // FillOrder
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 1;

	tags[tag].tagId = 274; // Orientation
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 1;

	tags[tag].tagId = 277; // SamplesPerPixel
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 3;

	tags[tag].tagId = 284; // PlanarConfiguration
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 1;

	tags[tag].tagId = 296; // ResolutionUnit
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = 1;

	tags[tag].tagId = 322; // TileWidth
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = TILE_SIZE;

	tags[tag].tagId = 323; // TileLength
	tags[tag].dataType = 0x03;
	tags[tag].dataCount = 0x01;
	tags[tag++].dataOffset = TILE_SIZE;

	tags[tag].tagId = 324; // TileOffsets
	tags[tag].dataType = 0x04;
	tags[tag].dataCount = TILE_COUNT;
	tags[tag++].dataOffset = 8 + (sizeof(short) * 3);

	tags[tag].tagId = 325; // TileByteCounts
	tags[tag].dataType = 0x04;
	tags[tag].dataCount = TILE_COUNT;
	tags[tag++].dataOffset = 8 + (sizeof(short) * 3) + (TILE_COUNT * sizeof(int));

	ifd.tagList = tags;

	return (ifd);
}

void	TIFF::arrayColorToDouble(const Image& image, std::vector<double>& doubleArray) const
{
	const unsigned int capacity = image.data().getCapacity();

	doubleArray.reserve(capacity * 3);

	for (unsigned int i = 0; i < capacity; i++)
	{
		doubleArray.push_back(image.data()[i].getRed());
		doubleArray.push_back(image.data()[i].getGreen());
		doubleArray.push_back(image.data()[i].getBlue());
	}
}
