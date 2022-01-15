#include "SceneFile/SceneFile.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include <fstream>

// Searches and reads / parses the Scene file named 'fileName' into 'Scene' (searches in the current directory)
void	SceneFile::read(Scene& scene, std::string fileName)
{
	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		std::cerr << CLR_RED << "The specified file could not be opened." << CLR_RESET << std::endl;
		exit(1);
	}

	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0 || line.at(0) == '#')
		{
			continue;
		}
		Utilities::toLower(line);

		if (line.rfind("[settings]", 0) != std::string::npos)
		{
			internal::_readSettingsSection(scene, stream);
		}
		else if (line.rfind("[scene]", 0) != std::string::npos)
		{
			internal::_readSceneSection(scene, stream);
		}
	} while (!stream.eof());
}
