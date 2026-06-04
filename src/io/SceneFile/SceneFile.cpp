#include "SceneFile/SceneFile.hpp"
#include "SceneFileInternal.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>

// Searches and reads / parses the Scene file named 'fileName' into 'Scene' (searches in the current directory)
void	SceneFile::read(Scene& scene, std::string fileName)
{
	scene.setIsFromFile(true);

	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		throw std::runtime_error("Scene file could not be opened: " + fileName);
	}

	const std::filesystem::path scenePath(fileName);
	const std::filesystem::path baseDirectory = scenePath.parent_path();

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
			internal::_readSceneSection(scene, stream, baseDirectory);
		}
	} while (!stream.eof());
}
