#include "SceneFileInternal.hpp"
#include "AssetPath.hpp"
#include "ColorManagement.hpp"
#include "ColorScience.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <sstream>
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
	return (_parseColorValue(value, label, std::filesystem::path()));
}

namespace SceneFile::internal
{
namespace
{
Color	parseColorValue(
	const std::string& value,
	const std::string& label,
	const std::filesystem::path& baseDirectory,
	const std::unordered_map<std::string, Color>* spectra
)
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
	auto parseSpectralFunctionArgument = [&](const std::string& functionName) -> std::string
	{
		const std::string prefix = functionName + "(";
		if (lower.rfind(prefix, 0) != 0 || lower.back() != ')')
		{
			throw std::runtime_error("Invalid " + label + " color function.");
		}

		std::string argument = _trim(trimmed.substr(prefix.length(), trimmed.length() - prefix.length() - 1));
		if (
			argument.length() >= 2
			&& (
				(argument.front() == '"' && argument.back() == '"')
				|| (argument.front() == '\'' && argument.back() == '\'')
			)
		)
		{
			argument = argument.substr(1, argument.length() - 2);
		}
		if (argument.empty())
		{
			throw std::runtime_error("Invalid " + label + " spectral curve reference.");
		}
		return (argument);
	};
	auto parseSpectralColor = [&](const std::string& functionName) -> Color
	{
		const std::string argument = parseSpectralFunctionArgument(functionName);

		if (spectra)
		{
			const auto spectrum = spectra->find(argument);
			if (spectrum != spectra->end())
			{
				return (spectrum->second);
			}
		}
		return (ColorScience::loadReflectanceCurve(_resolveAssetPath(baseDirectory, argument)));
	};
	auto parseColorFunction = [&](const std::string& functionName) -> Color
	{
		const std::string prefix = functionName + "(";
		if (lower.rfind(prefix, 0) != 0 || lower.back() != ')')
		{
			throw std::runtime_error("Invalid " + label + " color function.");
		}

		const std::string argument = _trim(trimmed.substr(prefix.length(), trimmed.length() - prefix.length() - 1));
		if (std::sscanf(argument.c_str(), "%lf,%lf,%lf", &r, &g, &b) != 3)
		{
			throw std::runtime_error("Invalid " + label + " color function. Use " + functionName + "(r,g,b).");
		}
		return (Color(r, g, b));
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
	if (lower.rfind("reflectance(", 0) == 0)
	{
		return (parseSpectralColor("reflectance"));
	}
	if (lower.rfind("reflectance_curve(", 0) == 0)
	{
		return (parseSpectralColor("reflectance_curve"));
	}
	if (lower.rfind("spectrum(", 0) == 0)
	{
		return (parseSpectralColor("spectrum"));
	}
	if (lower.rfind("spectral(", 0) == 0)
	{
		return (parseSpectralColor("spectral"));
	}
	if (lower.rfind("acescg(", 0) == 0)
	{
		return (parseColorFunction("acescg"));
	}
	if (
		lower.rfind("srgb(", 0) == 0
		|| lower.rfind("display_srgb(", 0) == 0
		|| lower.rfind("display(", 0) == 0
	)
	{
		if (lower.rfind("srgb(", 0) == 0)
		{
			return (ColorManagement::acescgFromSRGB(parseColorFunction("srgb")));
		}
		if (lower.rfind("display_srgb(", 0) == 0)
		{
			return (ColorManagement::acescgFromSRGB(parseColorFunction("display_srgb")));
		}
		return (ColorManagement::acescgFromSRGB(parseColorFunction("display")));
	}
	if (
		lower.rfind("linear_srgb(", 0) == 0
		|| lower.rfind("linearsrgb(", 0) == 0
		|| lower.rfind("lin_srgb(", 0) == 0
	)
	{
		if (lower.rfind("linear_srgb(", 0) == 0)
		{
			return (ColorManagement::acescgFromLinearSRGB(parseColorFunction("linear_srgb")));
		}
		if (lower.rfind("linearsrgb(", 0) == 0)
		{
			return (ColorManagement::acescgFromLinearSRGB(parseColorFunction("linearsrgb")));
		}
		return (ColorManagement::acescgFromLinearSRGB(parseColorFunction("lin_srgb")));
	}

	if (std::sscanf(trimmed.c_str(), "(%lf,%lf,%lf)", &r, &g, &b) != 3)
	{
		throw std::runtime_error("Invalid " + label + " color. Use " + label + "=(r,g,b), acescg(r,g,b), srgb(r,g,b), linear_srgb(r,g,b), wavelength(NM), blackbody(K), solar, or reflectance(PATH).");
	}

	return (Color(r, g, b));
}
}
}

Color	SceneFile::internal::_parseColorValue(
	const std::string& value,
	const std::string& label,
	const std::filesystem::path& baseDirectory
)
{
	return (parseColorValue(value, label, baseDirectory, nullptr));
}

Color	SceneFile::internal::_parseColorValue(
	const std::string& value,
	const std::string& label,
	const SceneFile::internal::SceneFileContext& context
)
{
	return (parseColorValue(value, label, context.baseDirectory, &context.spectra));
}

void	SceneFile::internal::_readNamedSpectraSection(std::ifstream& stream, SceneFileContext& context)
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

		std::string spectrumName;
		if (!_parseNamedBlockHeader(trimmedLine, "reflectance", spectrumName))
		{
			throw std::runtime_error("Invalid spectrum block header: " + line);
		}
		if (context.spectra.find(spectrumName) != context.spectra.end())
		{
			throw std::runtime_error("Duplicate spectrum name: " + spectrumName);
		}

		std::vector<ColorScience::SpectralSample> samples;
		do
		{
			getline(stream, line);
			std::size_t comment = line.find('#');
			if (comment != std::string::npos)
			{
				line = line.substr(0, comment);
			}
			std::replace(line.begin(), line.end(), ',', ' ');
			std::replace(line.begin(), line.end(), ';', ' ');
			const std::string sampleLine = _trim(line);

			if (sampleLine.empty())
			{
				continue;
			}
			if (sampleLine == "}")
			{
				try
				{
					context.spectra[spectrumName] = ColorScience::reflectanceCurve(samples);
				}
				catch (const std::exception& exception)
				{
					throw std::runtime_error("Invalid spectrum '" + spectrumName + "': " + exception.what());
				}
				break;
			}

			std::stringstream lineStream(sampleLine);
			ColorScience::SpectralSample sample;
			std::string trailing;
			if (!(lineStream >> sample.wavelengthNanometers >> sample.value) || (lineStream >> trailing))
			{
				throw std::runtime_error("Invalid spectrum sample in '" + spectrumName + "'. Use wavelength_nm,value.");
			}
			samples.push_back(sample);
		} while (!stream.eof());

		if (context.spectra.find(spectrumName) == context.spectra.end())
		{
			throw std::runtime_error("Spectrum block '" + spectrumName + "' is missing a closing }.");
		}
	} while (!stream.eof());
}
