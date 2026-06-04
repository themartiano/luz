#pragma once

#include "Materials/Material.hpp"
#include "Scene/Scene.hpp"
#include <filesystem>
#include <fstream>
#include <memory>

namespace SceneFile::internal
{
	void	_readSettingsSection(Scene& scene, std::ifstream& stream);
	void	_readSceneSection(Scene& scene, std::ifstream& stream, const std::filesystem::path& baseDirectory);
	void	_readObjectsSubSection(Scene& scene, std::ifstream& stream, const std::filesystem::path& baseDirectory);
	std::shared_ptr<Material>	_readMaterialSubSection(std::ifstream& stream);
}
