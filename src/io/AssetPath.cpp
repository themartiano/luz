#include "AssetPath.hpp"

std::string	AssetPath::resolve(const std::string& assetPath)
{
	return (resolve(std::filesystem::path(), assetPath));
}

std::string	AssetPath::resolve(const std::filesystem::path& baseDirectory, const std::string& assetPath)
{
	const std::filesystem::path path(assetPath);

	if (path.is_absolute())
	{
		return (path.string());
	}
	if (!baseDirectory.empty())
	{
		return ((baseDirectory / path).lexically_normal().string());
	}
	return (path.string());
}
