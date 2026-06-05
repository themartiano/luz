#include "SceneFileInternal.hpp"
#include <stdexcept>

void	SceneFile::internal::_readNamedMeshesSection(std::ifstream& stream, SceneFileContext& context)
{
	std::string line;

	do
	{
		getline(stream, line);
		const std::string trimmedLine = _trim(line);

		if (trimmedLine.empty())
		{
			break;
		}
		if (trimmedLine.at(0) == '#')
		{
			continue;
		}

		std::string meshName;
		if (!_parseNamedBlockHeader(trimmedLine, "mesh", meshName))
		{
			throw std::runtime_error("Invalid mesh block header: " + line);
		}
		if (context.meshes.find(meshName) != context.meshes.end())
		{
			throw std::runtime_error("Duplicate mesh name: " + meshName);
		}

		std::string meshFile;
		do
		{
			getline(stream, line);
			const std::string blockLine = _trim(line);

			if (blockLine.empty() || blockLine.at(0) == '#')
			{
				continue;
			}
			if (blockLine == "}")
			{
				if (meshFile.empty())
				{
					throw std::runtime_error("Mesh block '" + meshName + "' ended before file was defined.");
				}
				context.meshes[meshName] = _resolveAssetPath(context.baseDirectory, meshFile);
				break;
			}

			const std::size_t separator = blockLine.find('=');
			if (separator == std::string::npos || separator == 0 || separator == blockLine.length() - 1)
			{
				throw std::runtime_error("Invalid mesh property: " + blockLine);
			}

			const std::string key = _lowerCopy(_trim(blockLine.substr(0, separator)));
			const std::string value = _trim(blockLine.substr(separator + 1));
			if (key == "file" || key == "path")
			{
				meshFile = value;
			}
			else
			{
				throw std::runtime_error("Unknown mesh property: " + blockLine);
			}
		} while (!stream.eof());

		if (context.meshes.find(meshName) == context.meshes.end())
		{
			throw std::runtime_error("Mesh block '" + meshName + "' is missing a closing }.");
		}
	} while (!stream.eof());
}
