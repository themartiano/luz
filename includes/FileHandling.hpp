#ifndef FILEHANDLING_HPP
# define FILEHANDLING_HPP

#include "Scene.hpp"
#include "Exit.hpp"
#include <fstream>
#include <regex>
#include <iostream>
#include <string>

void	read_scene_file(Scene &scene, std::string fileName);

#endif