#include "FileHandling.hpp"

static void	resolution(Scene &scene, std::string line);

// Reads 'file' and stores values on 'scene'
void	read_scene_file(Scene &scene, std::string fileName)
{
	std::ifstream	file;

	file.open(fileName);
	if (!file.is_open())
		exit_error(scene, "The specified scene file could not be opened.");

	for (std::string line; std::getline(file, line);)
	{
		resolution(scene, line);

	}
}

static void	resolution(Scene &scene, std::string line)
{
	std::regex	resolution_regex("R [0-9]{1,5} [0-9]{1,5}$");
	std::smatch	match;

	if (std::regex_search(line, match, resolution_regex))
	{
		match[0].str().substr(2, match[0].str().)
		scene.setXResolution();
		// scene.setYResolution(match[2]);
	}
}
