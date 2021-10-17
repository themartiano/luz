#include "BMP.hpp"
#include <iostream>

static unsigned char*	createBitmapFileHeader(int height, int stride);
static unsigned char*	createBitmapInfoHeader(int height, int width);

BMP::BMP(void)
{
	this->_fileName = "";
}

BMP::BMP(std::string fileName)
{
	this->_fileName = fileName;
}

void	BMP::write_file(Scene scene)
{
	unsigned char	padding[3] = {0, 0, 0};
	int				paddingSize = (4 - (scene.getXResolution() * 3) % 4) % 4;
	int				stride = (scene.getXResolution() * 3) + paddingSize;
	unsigned char*	fileHeader = createBitmapFileHeader(scene.getYResolution(), stride);
	unsigned char*	infoHeader = createBitmapInfoHeader(scene.getYResolution(), scene.getXResolution());

	FILE* imageFile = fopen(this->_fileName.c_str(), "wb");

	fwrite(fileHeader, 1, 14, imageFile);
	fwrite(infoHeader, 1, 40, imageFile);

	for (int i = scene.getYResolution(); i >= 0; i--) {
		fwrite(scene.getPixelArray() + (i * scene.getXResolution() * 3), 3, scene.getXResolution(), imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

static unsigned char*	createBitmapFileHeader(int height, int stride)
{
	int fileSize = 14 + 40 + (stride * height);

	static unsigned char fileHeader[] = {
		0,0,		// Signature
		0,0,0,0,	// Image file size in bytes
		0,0,0,0,	// Reserved
		0,0,0,0,	// Start of pixel array
	};

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(14 + 40);

	return (fileHeader);
}

static unsigned char*	createBitmapInfoHeader(int height, int width)
{
	static unsigned char infoHeader[] = {
		0,0,0,0,	// Header size
		0,0,0,0,	// Image width
		0,0,0,0,	// Image height
		0,0,		// Number of color planes
		0,0,		// Bits per pixel
		0,0,0,0,	// Compression
		0,0,0,0,	// Image size
		0,0,0,0,	// Horizontal resolution
		0,0,0,0,	// Vertical resolution
		0,0,0,0,	// Colors in color table
		0,0,0,0,	// Important color count
	};

	infoHeader[0] = (unsigned char)(40);
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(3*8);

	return (infoHeader);
}
