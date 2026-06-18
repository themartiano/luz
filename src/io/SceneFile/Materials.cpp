#include "SceneFileInternal.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Principled.hpp"
#include "Materials/Isotropic.hpp"
#include "Materials/HenyeyGreenstein.hpp"
#include "Texture.hpp"
#include "LightUnits.hpp"
#include "RefractiveIndexes.hpp"
#include "Utilities.hpp"
#include <fstream>
#include <optional>
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
		double		refractiveIndex = RI_GLASS;
		double		alpha = 1.0;
		double		anisotropy = 0.0;
		std::optional<double>	emissionRadiance;
		std::optional<double>	emissionLuminance;
		std::optional<Color>	absorptionCoefficient;
		std::optional<Color>	transmittance;
		std::optional<Color>	conductorEta;
		std::optional<Color>	conductorExtinction;
		double		transmittanceDistance = 1.0;
		bool		hasTransmittanceDistance = false;
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
			double r, g, b;
			char extra;

			if (sscanf(lowerLine.c_str(), "emissive=(%lf,%lf,%lf)%c", &r, &g, &b, &extra) != 3)
			{
				throw std::runtime_error("Invalid emissive material: " + line);
			}
			material = std::make_shared<Emissive>(Color(r, g, b));
			return (true);
		}
		if (lowerLine.rfind("isotropic=", 0) != std::string::npos)
		{
			double r, g, b;

			if (sscanf(lowerLine.c_str(), "isotropic=(%lf,%lf,%lf)", &r, &g, &b) != 3)
			{
				throw std::runtime_error("Invalid isotropic material: " + line);
			}
			material = std::make_shared<Isotropic>(Color(r, g, b));
			return (true);
		}
		if (
			lowerLine.rfind("henyey_greenstein=", 0) != std::string::npos
			|| lowerLine.rfind("henyeygreenstein=", 0) != std::string::npos
			|| lowerLine.rfind("hg=", 0) != std::string::npos
		)
		{
			double r, g, b, anisotropy;

			if (sscanf(lowerLine.c_str(), "%*[^=]=(%lf,%lf,%lf),%lf", &r, &g, &b, &anisotropy) != 4)
			{
				throw std::runtime_error("Invalid Henyey-Greenstein material: " + line);
			}
			material = std::make_shared<HenyeyGreenstein>(Color(r, g, b), anisotropy);
			return (true);
		}

		return (false);
	}

	void	assignSingleEmissionQuantity(
		std::optional<double>& destination,
		const std::string& value,
		const MaterialBuilder& builder
	)
	{
		if (builder.emissionRadiance || builder.emissionLuminance)
		{
			throw std::runtime_error("Material block defines multiple emission unit quantities.");
		}
		destination = std::stod(value);
	}

	std::shared_ptr<Material>	buildEmissiveMaterial(
		const Color& color,
		const MaterialBuilder& builder
	)
	{
		if (builder.emissionRadiance)
		{
			return (std::make_shared<Emissive>(Emissive::fromRadiance(color, *builder.emissionRadiance)));
		}
		if (builder.emissionLuminance)
		{
			return (std::make_shared<Emissive>(Emissive::fromLuminance(color, *builder.emissionLuminance)));
		}
		return (std::make_shared<Emissive>(color));
	}

	std::shared_ptr<Dielectric>	buildDielectricMaterial(const MaterialBuilder& builder)
	{
		if (builder.absorptionCoefficient && builder.transmittance)
		{
			throw std::runtime_error("Dielectric material defines both absorption and transmittance.");
		}
		if (builder.hasTransmittanceDistance && !builder.transmittance)
		{
			throw std::runtime_error("Dielectric attenuation distance requires transmittance.");
		}

		std::shared_ptr<Dielectric> material = std::make_shared<Dielectric>(builder.color, builder.refractiveIndex);
		if (builder.absorptionCoefficient)
		{
			material->setAbsorptionCoefficient(*builder.absorptionCoefficient);
		}
		if (builder.transmittance)
		{
			material->setTransmittance(*builder.transmittance, builder.transmittanceDistance);
		}
		return (material);
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
		else if (key == "eta" || key == "conductor_eta")
		{
			builder.conductorEta = SceneFile::internal::_parseColorValue(value, key);
		}
		else if (
			key == "k"
			|| key == "extinction"
			|| key == "extinctioncoefficient"
			|| key == "extinction_coefficient"
			|| key == "conductor_k"
		)
		{
			builder.conductorExtinction = SceneFile::internal::_parseColorValue(value, key);
		}
		else if (key == "transmission")
		{
			builder.transmission = std::stod(value);
		}
		else if (key == "ior" || key == "refractiveindex" || key == "refractive_index")
		{
			builder.refractiveIndex = std::stod(value);
		}
		else if (
			key == "absorption"
			|| key == "absorptioncoefficient"
			|| key == "absorption_coefficient"
			|| key == "sigma_a"
			|| key == "sigmaa"
		)
		{
			builder.absorptionCoefficient = SceneFile::internal::_parseColorValue(value, key);
		}
		else if (
			key == "transmittance"
			|| key == "transmittancecolor"
			|| key == "transmittance_color"
			|| key == "attenuation"
			|| key == "attenuation_color"
			|| key == "attenuationcolor"
		)
		{
			builder.transmittance = SceneFile::internal::_parseColorValue(value, key);
		}
		else if (
			key == "attenuationdistance"
			|| key == "attenuation_distance"
			|| key == "absorptiondistance"
			|| key == "absorption_distance"
		)
		{
			builder.transmittanceDistance = std::stod(value);
			builder.hasTransmittanceDistance = true;
		}
		else if (key == "alpha")
		{
			builder.alpha = std::stod(value);
		}
		else if (
			key == "radiance"
			|| key == "surfaceradiance"
			|| key == "surface_radiance"
			|| key == "emissionradiance"
			|| key == "emission_radiance"
		)
		{
			assignSingleEmissionQuantity(builder.emissionRadiance, value, builder);
		}
		else if (
			key == "luminance"
			|| key == "nits"
			|| key == "cd_m2"
			|| key == "cdm2"
			|| key == "emissionluminance"
			|| key == "emission_luminance"
		)
		{
			assignSingleEmissionQuantity(builder.emissionLuminance, value, builder);
		}
		else if (key == "anisotropy" || key == "g")
		{
			builder.anisotropy = std::stod(value);
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

	std::shared_ptr<Metal>	buildMetalMaterial(const MaterialBuilder& builder)
	{
		if (builder.conductorEta || builder.conductorExtinction)
		{
			if (!builder.conductorEta || !builder.conductorExtinction)
			{
				throw std::runtime_error("Conductor metal requires both eta and k.");
			}
			return (std::make_shared<Metal>(
				*builder.conductorEta,
				*builder.conductorExtinction,
				builder.fuzz >= 0.0 ? builder.fuzz : builder.roughness
			));
		}
		return (std::make_shared<Metal>(builder.color, builder.fuzz >= 0.0 ? builder.fuzz : builder.roughness));
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
			if (builder.transmission > 0.0 || builder.alpha < 1.0)
			{
				std::shared_ptr<Material> material = buildDielectricMaterial(builder);
				attachTexture(material, builder, baseDirectory);
				return (material);
			}
			if (builder.metallic >= 0.5)
			{
				std::shared_ptr<Material> material = buildMetalMaterial(builder);
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
			std::shared_ptr<Material> material = buildMetalMaterial(builder);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "dielectric")
		{
			std::shared_ptr<Material> material = buildDielectricMaterial(builder);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "emissive")
		{
			return (buildEmissiveMaterial(
				builder.hasEmissionColor ? builder.emissionColor : builder.color,
				builder
			));
		}
		if (type == "isotropic")
		{
			return (std::make_shared<Isotropic>(builder.color));
		}
		if (
			type == "volume"
			|| type == "phase"
			|| type == "henyey_greenstein"
			|| type == "henyeygreenstein"
			|| type == "hg"
		)
		{
			return (std::make_shared<HenyeyGreenstein>(builder.color, builder.anisotropy));
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
