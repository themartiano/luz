#include "SceneFile.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include <fstream>

static void	readSettingsSection(Scene& scene, std::ifstream& stream);

// Searches and reads / parses the Scene file named 'fileName' into 'Scene' (searches in the current directory)
void   readSceneFile(Scene& scene, std::string fileName)
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

		if (line.length() <= 0)
		{
			continue;
		}
        Utilities::toLower(line);

        if (line.rfind("[settings]", 0) != std::string::npos)
        {
            readSettingsSection(scene, stream);
        }
        else if (line.rfind("[scene]", 0) != std::string::npos)
        {
            //readSceneSection(scene, stream);
        }
        else if (line.rfind("[materials]", 0) != std::string::npos)
        {
            //readMaterialsSection(scene, stream);
        }
	} while (!stream.eof());
}

// Parses the [settings] section of a Scene file
static void	readSettingsSection(Scene& scene, std::ifstream& stream)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			continue;
		}
        Utilities::toLower(line);

        if (line.rfind("resolution=", 0) != std::string::npos)
        {
            std::string temp = line.substr(line.find('=') + 1);
            scene.setXResolution(std::stoi(temp.substr(0, temp.find(','))));
            scene.setYResolution(std::stoi(temp.substr(temp.find(',') + 1)));
        }
        else if (line.rfind("samples=", 0) != std::string::npos)
        {
            scene.setSampleCount(std::stoi(line.substr(line.find('=') + 1)));
        }
        else if (line.rfind("maxlightbounces=", 0) != std::string::npos)
        {
            scene.setMaxLightBounces(std::stoi(line.substr(line.find('=') + 1)));
        }
        else if (line.rfind("gamma=", 0) != std::string::npos)
        {
            scene.setGammaCorrected(line.substr(line.find('=') + 1) == "true" ? true : false);
        }
	} while (!stream.eof());
}
