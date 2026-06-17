#include "SceneFile/SceneFile.hpp"
#include "SceneFileInternal.hpp"
#include "ANSIColors.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <thread>

namespace
{
	const std::size_t	MAX_CONCURRENT_MESH_LOADS = 4;

	std::size_t	countSceneMeshLoads(const std::string& fileName)
	{
		std::ifstream stream(fileName);
		std::string line;
		std::size_t meshLoads = 0;

		while (getline(stream, line))
		{
			const std::string lowerLine = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(line));

			if (lowerLine.empty() || lowerLine.at(0) == '#')
			{
				continue;
			}
			if (lowerLine.rfind("object ", 0) == 0 || lowerLine.rfind("obj=", 0) == 0)
			{
				meshLoads++;
			}
		}

		return (meshLoads);
	}

	std::size_t	meshLoadConcurrency(std::size_t meshLoads)
	{
		if (meshLoads <= 1)
		{
			return (1);
		}

		const unsigned int hardwareConcurrency = std::thread::hardware_concurrency();
		const std::size_t hardwareLimit = hardwareConcurrency == 0 ? 2 : hardwareConcurrency;
		const std::size_t cappedHardwareLimit = std::min(hardwareLimit, MAX_CONCURRENT_MESH_LOADS);

		return (std::max<std::size_t>(1, std::min(meshLoads, cappedHardwareLimit)));
	}
}

// Searches and reads / parses the Scene file named 'fileName' into 'Scene' (searches in the current directory)
void	SceneFile::read(Scene& scene, std::string fileName)
{
	scene.setIsFromFile(true);

	std::ifstream stream;
	stream.open(fileName);
	if (!stream)
	{
		throw std::runtime_error("Scene file could not be opened: " + fileName);
	}

	const std::filesystem::path scenePath(fileName);
	ObjLoadProgress meshLoadProgress;
	internal::SceneFileContext context;
	context.baseDirectory = scenePath.parent_path();
	meshLoadProgress.total = countSceneMeshLoads(fileName);
	context.meshLoadConcurrency = meshLoadConcurrency(meshLoadProgress.total);
	if (meshLoadProgress.total > 0)
	{
		context.meshLoadProgress = &meshLoadProgress;
	}

	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0 || line.at(0) == '#')
		{
			continue;
		}
		std::string lowerLine = line;
		Utilities::toLower(lowerLine);

		if (lowerLine.rfind("[settings]", 0) != std::string::npos)
		{
			internal::_readSettingsSection(scene, stream, context);
		}
		else if (lowerLine.rfind("[materials]", 0) != std::string::npos)
		{
			internal::_readNamedMaterialsSection(stream, context);
		}
		else if (lowerLine.rfind("[meshes]", 0) != std::string::npos)
		{
			internal::_readNamedMeshesSection(stream, context);
		}
		else if (lowerLine.rfind("[scene]", 0) != std::string::npos)
		{
			internal::_readSceneSection(scene, stream, context);
		}
		else
		{
			throw std::runtime_error("Unknown scene-file section: " + line);
		}
	} while (!stream.eof());

	for (const std::shared_future<std::shared_ptr<Hittable>>& pendingMeshLoad : context.pendingMeshLoads)
	{
		pendingMeshLoad.get();
	}
	scene.syncAtmosphereSunDirection();
}
