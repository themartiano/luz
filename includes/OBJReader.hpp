#pragma once

#include "Hittables/Mesh.hpp"
#include <string>

Mesh	readObj(std::string fileName);
Mesh	readObj(std::string fileName, Vector3 positionOffset, std::shared_ptr<Material> material);
