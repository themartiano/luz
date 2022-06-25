#include "ImageFiles/BMP.hpp"
#include "ANSIColors.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include <iostream>
#include <filesystem>
#include <utility>

// Static function prototypes
static unsigned char*	createBitmapFileHeader(int height, int stride);
static unsigned char*	createBitmapInfoHeader(int height, int width);
static unsigned char*	generateNewPixelArray(std::unique_ptr<Image> image);

BMP::BMP(void)
{
	this->_fileName = D_RENDER_FILE_NAME + ".bmp";
}

BMP::BMP(std::string fileName)
{
	this->_fileName = fileName;
}

// Writes a .bmp image file using the information present on 'scene'
void	BMP::writeFile(std::unique_ptr<Image> image, bool insideDir, std::string dirName)
{
	unsigned char	padding[3] = {0, 0, 0};
	int				paddingSize = (4 - (image->getWidth() * 3) % 4) % 4;
	int				stride = (image->getWidth() * 3) + paddingSize;
	unsigned char*	fileHeader = createBitmapFileHeader(image->getHeight(), stride);
	unsigned char*	infoHeader = createBitmapInfoHeader(image->getHeight(), image->getWidth());

	std::string filePath = "";
	if (insideDir == true)
	{
		std::filesystem::create_directory(dirName);
		filePath = dirName + "/";
	}
	filePath += this->_fileName;

	if (!Utilities::stringEndsWith(filePath, ".bmp"))
	{
		filePath += ".bmp";
	}

	std::cout << CLR_YELLOW << "Writing render to " << CLR_BLUE_BRIGHT << filePath << CLR_YELLOW << "...\n" << CLR_RESET;

	FILE* imageFile = fopen(filePath.c_str(), "wb");

	fwrite(fileHeader, 1, 14, imageFile);
	fwrite(infoHeader, 1, 40, imageFile);

	unsigned char*	newPixelArray = generateNewPixelArray(std::move(image));

	for (int i = image->getHeight() - 1; i >= 0; i--) {
		fwrite(newPixelArray + (i * image->getWidth() * 3), 3, image->getWidth(), imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	delete[] newPixelArray;

	fclose(imageFile);
	std::cout << CLR_GREEN_BRIGHT << "File ready.\n\n" << CLR_RESET;
}

// BMP::writeFile overload
void	BMP::writeFile(std::unique_ptr<Image> image)
{
	writeFile(std::move(image), false, "");
}

static unsigned char*	generateNewPixelArray(std::unique_ptr<Image> image)
{
	unsigned char* newPixelArray = new unsigned char[image->getWidth() * image->getHeight() * 3];

	for (std::size_t y = 0; y < image->getHeight(); y++)
	{
		for (std::size_t x = 0; x < image->getWidth(); x++)
		{
			Color pixel = image->getPixel(x, y);

			double r = pixel.getRed();
			double g = pixel.getGreen();
			double b = pixel.getBlue();

			Utilities::setDoubleRange(r, 0.0, 1.0);
			Utilities::setDoubleRange(g, 0.0, 1.0);
			Utilities::setDoubleRange(b, 0.0, 1.0);

			std::size_t index = (y * image->getHeight()) + x;

			newPixelArray[(index * 3) + 2] = (unsigned char)(r * 255.0);
			newPixelArray[(index * 3) + 1] = (unsigned char)(g * 255.0);
			newPixelArray[(index * 3) + 0] = (unsigned char)(b * 255.0);
		}
	}

	return (newPixelArray);
}

// Creates and returns the BMP image "file header"
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

// Creates and returns the BMP image "info header"
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
