#include "SceneFileInternal.hpp"
#include "AssetPath.hpp"
#include "Utilities.hpp"
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <stdexcept>

std::string	SceneFile::internal::_trim(const std::string& input)
{
	std::size_t start = 0;
	std::size_t end = input.size();

	while (start < end && std::isspace(static_cast<unsigned char>(input[start])))
	{
		start++;
	}
	while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1])))
	{
		end--;
	}

	return (input.substr(start, end - start));
}

std::string	SceneFile::internal::_lowerCopy(std::string input)
{
	Utilities::toLower(input);

	return (input);
}

std::string	SceneFile::internal::_resolveAssetPath(const std::filesystem::path& baseDirectory, const std::string& assetPath)
{
	return (AssetPath::resolve(baseDirectory, assetPath));
}

bool	SceneFile::internal::_parseNamedBlockHeader(const std::string& line, const std::string& keyword, std::string& name)
{
	const std::string trimmedLine = _trim(line);
	const std::string prefix = keyword + " ";

	if (trimmedLine.rfind(prefix, 0) != 0)
	{
		return (false);
	}
	if (trimmedLine.empty() || trimmedLine.back() != '{')
	{
		return (false);
	}

	const std::string rawName = _trim(trimmedLine.substr(prefix.size(), trimmedLine.size() - prefix.size() - 1));
	if (rawName.empty() || rawName.find_first_of(" \t{}") != std::string::npos)
	{
		return (false);
	}

	name = rawName;

	return (true);
}

Vector3	SceneFile::internal::_parseVector3Value(const std::string& value, const std::string& label)
{
	double x;
	double y;
	double z;

	if (std::sscanf(_trim(value).c_str(), "(%lf,%lf,%lf)", &x, &y, &z) != 3)
	{
		throw std::runtime_error("Invalid " + label + " vector. Use " + label + "=(x,y,z).");
	}

	return (Vector3(x, y, z));
}

Color	SceneFile::internal::_parseColorValue(const std::string& value, const std::string& label)
{
	double r;
	double g;
	double b;

	if (std::sscanf(_trim(value).c_str(), "(%lf,%lf,%lf)", &r, &g, &b) != 3)
	{
		throw std::runtime_error("Invalid " + label + " color. Use " + label + "=(r,g,b).");
	}

	return (Color(r, g, b));
}
