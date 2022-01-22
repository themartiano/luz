#pragma once

#include "Scene.hpp"
#include <string>
#include <memory>

namespace	SceneFile
{
	void	read(Scene& scene, std::string fileName);

	namespace	internal
	{
		void	_readSettingsSection(Scene& scene, std::ifstream& stream);
		void	_readSceneSection(Scene& scene, std::ifstream& stream);
		void	_readObjectsSubSection(Scene& scene, std::ifstream& stream);
		std::shared_ptr<Material>	_readMaterialSubSection(std::ifstream& stream);
	}
}
