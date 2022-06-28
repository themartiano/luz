#pragma once

#include "Scene.hpp"
#include <string>
#include <memory>

class	TIFF
{
	public:
		TIFF(void);
		TIFF(std::string fileName);
		void	writeFile(const std::unique_ptr<Image>& image, bool insideDir, std::string dirName);
		void	writeFile(const std::unique_ptr<Image>& image);

		#pragma pack(push, 1)
		typedef struct	Header
		{
			short	byteOrderIdentifier = 0x4949;
			short	version = 0x2a;
			int		IFDOffset;
		}	tiffHeader;

		typedef struct	Tag
		{
			short	tagId;
			short	dataType;
			int		dataCount;
			int		dataOffset;
		}	tiffTag;

		typedef struct	IFD
		{
			short	tagCount;
			tiffTag	*tagList;
			int		nextIFDOffset = 0x00;
		}	tiffIFD;
		#pragma pack(pop)

	private:
		void	arrayColorToDouble(const std::unique_ptr<Image>& image, std::vector<double>& doubleArray) const;

		std::string	_fileName;
		static tiffIFD	_generateIFD(void);
};
