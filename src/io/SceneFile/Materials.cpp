#include "SceneFileInternal.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Principled.hpp"
#include "Texture.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstdio>

namespace
{
	struct MaterialBuilder
	{
		std::string	type;
		Color		color = Color(0.6, 0.6, 0.6);
		Color		emissionColor = Color(1.0, 1.0, 1.0);
		double		fuzz = -1.0;
		double		roughness = 0.0;
		double		metallic = 0.0;
		double		transmission = 0.0;
		double		alpha = 1.0;
		double		intensity = 1.0;
		double		emissionStrength = 0.0;
		std::string	texturePath;
		bool		hasProperties = false;
		bool		hasEmissionColor = false;
		std::shared_ptr<Material>	directMaterial = nullptr;
	};

	bool	splitAssignment(const std::string& line, std::string& key, std::string& value)
	{
		const std::size_t separator = line.find('=');

		if (separator == std::string::npos || separator == 0 || separator == line.length() - 1)
		{
			return (false);
		}
		key = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(line.substr(0, separator)));
		value = SceneFile::internal::_trim(line.substr(separator + 1));

		return (true);
	}

	bool	parseDirectMaterialLine(const std::string& line, std::shared_ptr<Material>& material)
	{
		const std::string lowerLine = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(line));

		if (lowerLine.rfind("lambertian=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(lowerLine.c_str(), "lambertian=(%lf,%lf,%lf)", &r, &g, &b) != 3)
			{
				throw std::runtime_error("Invalid lambertian material: " + line);
			}
			material = std::make_shared<Lambertian>(Color(r, g, b));
			return (true);
		}
		if (lowerLine.rfind("metal=", 0) != std::string::npos)
		{
			double r, g, b, reflectionFuzziness;

			if (sscanf(lowerLine.c_str(), "metal=(%lf,%lf,%lf),%lf", &r, &g, &b, &reflectionFuzziness) != 4)
			{
				throw std::runtime_error("Invalid metal material: " + line);
			}
			material = std::make_shared<Metal>(Color(r, g, b), reflectionFuzziness);
			return (true);
		}
		if (lowerLine.rfind("dielectric=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(lowerLine.c_str(), "dielectric=(%lf,%lf,%lf)", &r, &g, &b) != 3)
			{
				throw std::runtime_error("Invalid dielectric material: " + line);
			}
			material = std::make_shared<Dielectric>(Color(r, g, b));
			return (true);
		}
		if (lowerLine.rfind("emissive=", 0) != std::string::npos)
		{
			double r, g, b, lightIntensity;

			if (sscanf(lowerLine.c_str(), "emissive=(%lf,%lf,%lf),%lf", &r, &g, &b, &lightIntensity) != 4)
			{
				throw std::runtime_error("Invalid emissive material: " + line);
			}
			material = std::make_shared<Emissive>(Color(r, g, b), lightIntensity);
			return (true);
		}

		return (false);
	}

	bool	colorHasEnergy(const Color& color)
	{
		return (
			color.getRed() > 0.0
			|| color.getGreen() > 0.0
			|| color.getBlue() > 0.0
		);
	}

	void	parseMaterialProperty(MaterialBuilder& builder, const std::string& line)
	{
		std::string key;
		std::string value;

		if (!splitAssignment(line, key, value))
		{
			throw std::runtime_error("Invalid material property: " + line);
		}
		if (builder.directMaterial)
		{
			throw std::runtime_error("Material block mixes direct material syntax with property syntax.");
		}

		builder.hasProperties = true;
		if (key == "type")
		{
			builder.type = SceneFile::internal::_lowerCopy(value);
		}
		else if (key == "color" || key == "basecolor" || key == "base_color")
		{
			builder.color = SceneFile::internal::_parseColorValue(value, key);
		}
		else if (key == "emission" || key == "emissioncolor" || key == "emission_color")
		{
			builder.emissionColor = SceneFile::internal::_parseColorValue(value, key);
			builder.hasEmissionColor = true;
		}
		else if (key == "fuzz")
		{
			builder.fuzz = std::stod(value);
		}
		else if (key == "roughness")
		{
			builder.roughness = std::stod(value);
		}
		else if (key == "metallic")
		{
			builder.metallic = std::stod(value);
		}
		else if (key == "transmission")
		{
			builder.transmission = std::stod(value);
		}
		else if (key == "alpha")
		{
			builder.alpha = std::stod(value);
		}
		else if (key == "intensity")
		{
			builder.intensity = std::stod(value);
		}
		else if (key == "emissionstrength" || key == "emission_strength")
		{
			builder.emissionStrength = std::stod(value);
		}
		else if (key == "texture" || key == "basecolortexture" || key == "base_color_texture" || key == "albedo")
		{
			builder.texturePath = value;
		}
		else
		{
			throw std::runtime_error("Unknown material property: " + line);
		}
	}

	void	attachTexture(
		std::shared_ptr<Material> material,
		const MaterialBuilder& builder,
		const std::filesystem::path& baseDirectory
	)
	{
		if (!builder.texturePath.empty())
		{
			material->setTexture(std::make_shared<Texture>(
				Texture::loadPPM(SceneFile::internal::_resolveAssetPath(baseDirectory, builder.texturePath))
			));
		}
	}

	std::shared_ptr<Material>	buildMaterial(
		const MaterialBuilder& builder,
		const std::string& blockDescription,
		const std::filesystem::path& baseDirectory
	)
	{
		if (builder.directMaterial)
		{
			attachTexture(builder.directMaterial, builder, baseDirectory);
			return (builder.directMaterial);
		}
		if (!builder.hasProperties)
		{
			throw std::runtime_error(blockDescription + " ended before a material was defined.");
		}

		std::string type = builder.type.empty() ? "lambertian" : builder.type;
		if (type == "principled")
		{
			const Color emissionColor = builder.hasEmissionColor ? builder.emissionColor : builder.color;
			if (builder.emissionStrength > 0.0 && colorHasEnergy(emissionColor))
			{
				return (std::make_shared<Emissive>(emissionColor, builder.emissionStrength));
			}
			if (builder.transmission > 0.0 || builder.alpha < 1.0)
			{
				std::shared_ptr<Material> material = std::make_shared<Dielectric>(builder.color);
				attachTexture(material, builder, baseDirectory);
				return (material);
			}
			if (builder.metallic >= 0.5)
			{
				std::shared_ptr<Material> material = std::make_shared<Metal>(builder.color, builder.fuzz >= 0.0 ? builder.fuzz : builder.roughness);
				attachTexture(material, builder, baseDirectory);
				return (material);
			}
			std::shared_ptr<Material> material = std::make_shared<Principled>(builder.color, builder.metallic, builder.roughness);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "lambertian")
		{
			std::shared_ptr<Material> material = std::make_shared<Lambertian>(builder.color);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "metal")
		{
			std::shared_ptr<Material> material = std::make_shared<Metal>(builder.color, builder.fuzz >= 0.0 ? builder.fuzz : builder.roughness);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "dielectric")
		{
			std::shared_ptr<Material> material = std::make_shared<Dielectric>(builder.color);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "emissive")
		{
			return (std::make_shared<Emissive>(
				builder.hasEmissionColor ? builder.emissionColor : builder.color,
				builder.emissionStrength > 0.0 ? builder.emissionStrength : builder.intensity
			));
		}

		throw std::runtime_error("Unknown material type: " + type);
	}

	std::shared_ptr<Material>	readBraceMaterialBlock(
		std::ifstream& stream,
		const std::string& blockDescription,
		const std::filesystem::path& baseDirectory
	)
	{
		std::string line;
		MaterialBuilder builder;

		do
		{
			getline(stream, line);
			const std::string trimmedLine = SceneFile::internal::_trim(line);

			if (trimmedLine.empty() || trimmedLine.at(0) == '#')
			{
				continue;
			}
			if (trimmedLine == "}")
			{
				return (buildMaterial(builder, blockDescription, baseDirectory));
			}

			std::shared_ptr<Material> directMaterial;
			if (parseDirectMaterialLine(trimmedLine, directMaterial))
			{
				if (builder.directMaterial || builder.hasProperties)
				{
					throw std::runtime_error("Material block defines more than one material.");
				}
				builder.directMaterial = directMaterial;
				continue;
			}

			parseMaterialProperty(builder, trimmedLine);
		} while (!stream.eof());

		throw std::runtime_error(blockDescription + " is missing a closing }.");
	}
}

// Parses a Material from a Scene file
std::shared_ptr<Material>	SceneFile::internal::_readMaterialSubSection(std::ifstream& stream)
{
	std::string line;
	std::shared_ptr<Material> material;

	do
	{
		getline(stream, line);
		const std::string trimmedLine = _trim(line);

		if (trimmedLine.empty() || trimmedLine.at(0) == '#')
		{
			continue;
		}
		if (trimmedLine == "]")
		{
			if (!material)
			{
				throw std::runtime_error("Material block ended before a material was defined.");
			}
			return (material);
		}

		std::shared_ptr<Material> parsedMaterial;
		if (!parseDirectMaterialLine(trimmedLine, parsedMaterial))
		{
			throw std::runtime_error("Unknown material line: " + line);
		}
		if (material)
		{
			throw std::runtime_error("Material block defines more than one material.");
		}
		material = parsedMaterial;
	} while (!stream.eof());

	throw std::runtime_error("Material block is missing a closing ].");
}

void	SceneFile::internal::_readNamedMaterialsSection(std::ifstream& stream, SceneFileContext& context)
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

		std::string materialName;
		if (!_parseNamedBlockHeader(trimmedLine, "material", materialName))
		{
			throw std::runtime_error("Invalid material block header: " + line);
		}
		if (context.materials.find(materialName) != context.materials.end())
		{
			throw std::runtime_error("Duplicate material name: " + materialName);
		}

		context.materials[materialName] = readBraceMaterialBlock(
			stream,
			"Material block '" + materialName + "'",
			context.baseDirectory
		);
	} while (!stream.eof());
}
