#pragma once

#include <filesystem>
#include <string>

namespace AssetPath
{
	std::string	resolve(const std::string& assetPath);
	std::string	resolve(const std::filesystem::path& baseDirectory, const std::string& assetPath);
}
