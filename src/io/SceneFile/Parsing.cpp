#include "SceneFileInternal.hpp"
#include "AssetPath.hpp"
#include "ColorScience.hpp"
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
	const std::string trimmed = _trim(value);
	const std::string lower = _lowerCopy(trimmed);

	if (trimmed.empty())
	{
		throw std::runtime_error("Invalid " + label + " color. Color value is empty.");
	}

	auto parseFunctionArgument = [&](const std::string& functionName, const std::string& suffix) -> double
	{
		const std::string prefix = functionName + "(";
		if (lower.rfind(prefix, 0) != 0 || lower.back() != ')')
		{
			throw std::runtime_error("Invalid " + label + " color function.");
		}

		std::string argument = _trim(trimmed.substr(prefix.length(), trimmed.length() - prefix.length() - 1));
		std::string lowerArgument = _lowerCopy(argument);
		if (
			!suffix.empty()
			&& lowerArgument.length() > suffix.length()
			&& lowerArgument.compare(lowerArgument.length() - suffix.length(), suffix.length(), suffix) == 0
		)
		{
			argument = _trim(argument.substr(0, argument.length() - suffix.length()));
			lowerArgument = _lowerCopy(argument);
		}

		std::size_t parsed = 0;
		const double scalar = std::stod(argument, &parsed);
		if (parsed != argument.length() || !std::isfinite(scalar))
		{
			throw std::runtime_error("Invalid " + label + " color function value.");
		}
		return (scalar);
	};

	if (lower.rfind("wavelength(", 0) == 0)
	{
		return (ColorScience::wavelength(parseFunctionArgument("wavelength", "nm")));
	}
	if (lower.rfind("blackbody(", 0) == 0)
	{
		return (ColorScience::blackbody(parseFunctionArgument("blackbody", "k")));
	}
	if (lower.rfind("temperature(", 0) == 0)
	{
		return (ColorScience::blackbody(parseFunctionArgument("temperature", "k")));
	}
	if (lower.rfind("color_temperature(", 0) == 0)
	{
		return (ColorScience::blackbody(parseFunctionArgument("color_temperature", "k")));
	}
	if (lower == "solar" || lower == "sun")
	{
		return (ColorScience::solar());
	}

	if (std::sscanf(trimmed.c_str(), "(%lf,%lf,%lf)", &r, &g, &b) != 3)
	{
		throw std::runtime_error("Invalid " + label + " color. Use " + label + "=(r,g,b), " + label + "=wavelength(NM), " + label + "=blackbody(K), or " + label + "=solar.");
	}

	return (Color(r, g, b));
}
