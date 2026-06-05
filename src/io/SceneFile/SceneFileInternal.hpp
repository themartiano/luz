#pragma once

#include "Materials/Material.hpp"
#include "Scene/Scene.hpp"
#include "Color.hpp"
#include "Vector3.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

namespace SceneFile::internal
{
	struct SceneFileContext
	{
		std::filesystem::path	baseDirectory;
		std::unordered_map<std::string, std::shared_ptr<Material>>	materials;
		std::unordered_map<std::string, std::string>	meshes;
	};

	void	_readSettingsSection(Scene& scene, std::ifstream& stream);
	void	_readNamedMaterialsSection(std::ifstream& stream, SceneFileContext& context);
	void	_readNamedMeshesSection(std::ifstream& stream, SceneFileContext& context);
	void	_readSceneSection(Scene& scene, std::ifstream& stream, const SceneFileContext& context);
	void	_readObjectsSubSection(Scene& scene, std::ifstream& stream, const SceneFileContext& context);
	bool	_readSceneObjectOrLightBlock(Scene& scene, std::ifstream& stream, const SceneFileContext& context, const std::string& line);
	std::shared_ptr<Material>	_readMaterialSubSection(std::ifstream& stream);
	std::string	_trim(const std::string& input);
	std::string	_lowerCopy(std::string input);
	std::string	_resolveAssetPath(const std::filesystem::path& baseDirectory, const std::string& assetPath);
	bool	_parseNamedBlockHeader(const std::string& line, const std::string& keyword, std::string& name);
	Vector3	_parseVector3Value(const std::string& value, const std::string& label);
	Color	_parseColorValue(const std::string& value, const std::string& label);
}
