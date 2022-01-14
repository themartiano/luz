#pragma once

#include "Scene.hpp"
#include <string>

class	TIFF
{
	public:
		TIFF(void);
		TIFF(std::string fileName);
		void	writeFile(Scene& scene, bool insideDir, std::string dirName);
		void	writeFile(Scene& scene);

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
		std::string	_fileName;
		static tiffIFD	_generateIFD(void);
};
