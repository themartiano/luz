#include "AssetPath.hpp"
#include <vector>

namespace
{
	std::vector<std::filesystem::path>	assetCandidates(const std::filesystem::path& path)
	{
		return (std::vector<std::filesystem::path>{
			path,
			std::filesystem::path("textures") / path,
			std::filesystem::path("assets/textures") / path,
			std::filesystem::path("assets/objects") / path,
			std::filesystem::path("objects") / path
		});
	}

	bool	resolveFromDirectory(
		std::filesystem::path directory,
		const std::vector<std::filesystem::path>& candidates,
		std::string& resolvedPath
	)
	{
		while (true)
		{
			for (const std::filesystem::path& candidate : candidates)
			{
				const std::filesystem::path path = directory / candidate;

				if (std::filesystem::exists(path))
				{
					resolvedPath = path.string();
					return (true);
				}
			}
			if (directory == directory.root_path())
			{
				return (false);
			}
			directory = directory.parent_path();
		}
	}
}

std::string	AssetPath::resolve(const std::string& assetPath)
{
	return (resolve(std::filesystem::path(), assetPath));
}

std::string	AssetPath::resolve(const std::filesystem::path& baseDirectory, const std::string& assetPath)
{
	const std::filesystem::path path(assetPath);
	std::string resolvedPath;

	if (path.is_absolute())
	{
		return (path.string());
	}

	if (!baseDirectory.empty())
	{
		const std::filesystem::path sceneRelativePath = baseDirectory / path;
		if (std::filesystem::exists(sceneRelativePath))
		{
			return (sceneRelativePath.string());
		}
	}

	const std::vector<std::filesystem::path> candidates = assetCandidates(path);
	if (resolveFromDirectory(std::filesystem::current_path(), candidates, resolvedPath))
	{
		return (resolvedPath);
	}
	if (!baseDirectory.empty() && resolveFromDirectory(baseDirectory, candidates, resolvedPath))
	{
		return (resolvedPath);
	}
	if (!baseDirectory.empty())
	{
		return ((baseDirectory / path).string());
	}
	return (path.string());
}
