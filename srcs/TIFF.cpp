#include "TIFF.hpp"
#include "ANSIColors.hpp"
#include <iostream>
#include <filesystem>

#define RESOLUTION 256
#define TILE_SIZE 16 // multiple of 16
#define TILE_COUNT ((RESOLUTION / TILE_SIZE) * (RESOLUTION / TILE_SIZE))

// Writes a .tiff image file using the information present on 'scene'
void	TIFF::writeFile(Scene& scene, bool insideDir, std::string dirName)
{
	std::cout << CLR_YELLOW << "Writing render to " << CLR_BLUE_BRIGHT << scene.getOutputFileName() << CLR_YELLOW << "...\n" << CLR_RESET;

	std::string filePath = scene.getOutputFileName();
	if (insideDir == true)
	{
		std::filesystem::create_directory(dirName);
		filePath = dirName + "/" + scene.getOutputFileName();
	}

	FILE* imageFile = fopen(filePath.c_str(), "wb");
	tiffIFD ifd = _generateIFD();

	// Write header
	tiffHeader header;
	header.IFDOffset = sizeof(tiffHeader) + (sizeof(short) * 3) + (TILE_COUNT * sizeof(int) * 2);
	fwrite(&header, sizeof(tiffHeader), 1, imageFile);

	// Write bits per sample
	short	bits = 8;
	for (int i = 0; i < 3; i++)
	{
		fwrite(&bits, sizeof(short), 1, imageFile);
	}

	// Write tile offsets
	int	baseOffset = sizeof(tiffHeader) + (sizeof(short) * 3) + (TILE_COUNT * sizeof(int) * 2) + (sizeof(short) + (sizeof(tiffTag) * ifd.tagCount) + sizeof(int));
	for (int i = 0; i < TILE_COUNT; i++)
	{
		int offset = baseOffset + (i * TILE_SIZE * TILE_SIZE * 3);
		fwrite(&offset, sizeof(int), 1, imageFile);
	}

	// Write tile byte count
	int	tileByteCount = TILE_SIZE * TILE_SIZE * 3;
	for (int i = 0; i < TILE_COUNT; i++)
	{
		fwrite(&tileByteCount, sizeof(int), 1, imageFile);
	}

	// Write IFD
	fwrite(&ifd.tagCount, sizeof(short), 1, imageFile);
	fwrite(ifd.tagList, sizeof(tiffTag), ifd.tagCount, imageFile);
	fwrite(&ifd.nextIFDOffset, sizeof(int), 1, imageFile);
	delete[] ifd.tagList;

	// Write bitmap image data
	//fwrite(scene.getPixelArray(), sizeof(unsigned char) * 3, scene.getXResolution() * scene.getYResolution(), imageFile);
	for (int row = 0; row < RESOLUTION / TILE_SIZE; row++)
	{
		int rowPos = (row * TILE_SIZE * scene.getXResolution() * 3);
		for (int column = 0; column < RESOLUTION / TILE_SIZE; column++)
		{
			int columnPos = (column * TILE_SIZE * 3);
			for (int y = 0; y < TILE_SIZE; y++)
			{
				int tilePos = (y * scene.getXResolution() * 3);
				fwrite(scene.getPixelArray() + rowPos + columnPos + tilePos, 3, TILE_SIZE, imageFile);
			}
		}
	}

	fclose(imageFile);
	std::cout << CLR_GREEN_BRIGHT << "File ready.\n\n" << CLR_RESET;
}

// TIFF::writeFile overload
void	TIFF::writeFile(Scene& scene)
{
	writeFile(scene, false, "");
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
