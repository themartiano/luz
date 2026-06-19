#include "SceneFileInternal.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Dielectric.hpp"
#include "Materials/Emissive.hpp"
#include "Materials/Principled.hpp"
#include "Materials/Glossy.hpp"
#include "Materials/DiffuseGlossy.hpp"
#include "Materials/Isotropic.hpp"
#include "Materials/HenyeyGreenstein.hpp"
#include "Texture.hpp"
#include "LightUnits.hpp"
#include "MeasuredMaterials.hpp"
#include "RefractiveIndexes.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <array>
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
		bool		hasRefractiveIndex = false;
		double		clearcoat = 0.0;
		double		clearcoatRoughness = 0.03;
		double		sheen = 0.0;
		double		subsurface = 0.0;
		Color		subsurfaceRadius = Color(1.0, 1.0, 1.0);
		double		subsurfaceScale = 0.001;
		Color		subsurfaceColor = Color(1.0, 1.0, 1.0);
		SubsurfaceMethod	subsurfaceMethod = SUBSURFACE_BURLEY;
		bool		hasSubsurface = false;
		bool		hasSubsurfaceRadius = false;
		bool		hasSubsurfaceScale = false;
		bool		hasSubsurfaceColor = false;
		bool		hasSubsurfaceMethod = false;
		std::string	subsurfaceProfile;
		Color		glossyColor = Color(1.0, 1.0, 1.0);
		double		glossyWeight = 0.5;
		bool		hasGlossyColor = false;
		double		alpha = 1.0;
		double		anisotropy = 0.0;
		std::optional<double>	emissionRadiance;
		std::optional<double>	emissionLuminance;
		std::optional<Color>	absorptionCoefficient;
		std::optional<Color>	transmittance;
		std::optional<Color>	conductorEta;
		std::optional<Color>	conductorExtinction;
		std::string	materialPreset;
		std::string	conductorPreset;
		std::string	glassPreset;
		std::optional<double>	abbeNumber;
		std::optional<Vector3>	sellmeierB;
		std::optional<Vector3>	sellmeierC;
		double		iorWavelengthNanometers = 587.5618;
		bool		hasIorWavelength = false;
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

	std::array<double, 3>	vectorToArray(Vector3 vector)
	{
		return (std::array<double, 3>{vector.getX(), vector.getY(), vector.getZ()});
	}

	double	parseNanometers(std::string value, const std::string& label)
	{
		value = SceneFile::internal::_trim(value);
		std::string lowerValue = SceneFile::internal::_lowerCopy(value);
		if (
			lowerValue.length() > 2
			&& lowerValue.compare(lowerValue.length() - 2, 2, "nm") == 0
		)
		{
			value = SceneFile::internal::_trim(value.substr(0, value.length() - 2));
		}

		std::size_t parsed = 0;
		const double nanometers = std::stod(value, &parsed);
		if (parsed != value.length() || !std::isfinite(nanometers) || nanometers <= 0.0)
		{
			throw std::runtime_error(label + " must be a finite positive wavelength in nm.");
		}
		return (nanometers);
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
		if (lowerLine.rfind("glossy=", 0) != std::string::npos)
		{
			double r, g, b, roughness;

			if (sscanf(lowerLine.c_str(), "glossy=(%lf,%lf,%lf),%lf", &r, &g, &b, &roughness) != 4)
			{
				throw std::runtime_error("Invalid glossy material: " + line);
			}
			material = std::make_shared<Glossy>(Color(r, g, b), roughness);
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
			throw std::runtime_error("Direct emissive material syntax is not physical. Use type=emissive with radiance or luminance.");
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
		throw std::runtime_error("Emissive material requires radiance or luminance.");
	}

	std::optional<MeasuredMaterials::Glass>	selectedGlassPreset(
		const MaterialBuilder& builder,
		const std::string& type
	)
	{
		if (!builder.glassPreset.empty())
		{
			return (MeasuredMaterials::glassPreset(builder.glassPreset));
		}
		if (
			!builder.materialPreset.empty()
			&& (
				type == "dielectric"
				|| type == "principled"
			)
		)
		{
			return (MeasuredMaterials::glassPreset(builder.materialPreset));
		}
		return (std::nullopt);
	}

	double	materialRefractiveIndex(const MaterialBuilder& builder, const std::string& type)
	{
		const std::optional<MeasuredMaterials::Glass> glass = selectedGlassPreset(builder, type);
		if (glass && builder.hasRefractiveIndex)
		{
			throw std::runtime_error("Material defines both glass preset and explicit ior.");
		}
		if (glass && builder.abbeNumber)
		{
			throw std::runtime_error("Material defines both glass preset and explicit Abbe number.");
		}
		if (glass && (builder.sellmeierB || builder.sellmeierC))
		{
			throw std::runtime_error("Material defines both glass preset and explicit Sellmeier coefficients.");
		}
		if (builder.abbeNumber && (builder.sellmeierB || builder.sellmeierC))
		{
			throw std::runtime_error("Material defines both Abbe and Sellmeier dispersion.");
		}
		if (builder.sellmeierB || builder.sellmeierC)
		{
			if (!builder.sellmeierB || !builder.sellmeierC)
			{
				throw std::runtime_error("Sellmeier dispersion requires both sellmeier_b and sellmeier_c.");
			}
			return (MeasuredMaterials::refractiveIndexFromSellmeier(
				vectorToArray(*builder.sellmeierB),
				vectorToArray(*builder.sellmeierC),
				builder.iorWavelengthNanometers
			));
		}
		if (glass && glass->hasSellmeier)
		{
			return (MeasuredMaterials::refractiveIndexFromSellmeier(
				glass->sellmeierB,
				glass->sellmeierC,
				builder.iorWavelengthNanometers
			));
		}
		if (builder.abbeNumber)
		{
			const double nd = glass ? glass->refractiveIndexD : builder.refractiveIndex;
			return (MeasuredMaterials::refractiveIndexFromAbbe(
				nd,
				*builder.abbeNumber,
				builder.iorWavelengthNanometers
			));
		}
		if (glass)
		{
			if (builder.hasIorWavelength && glass->abbeNumber > 0.0)
			{
				return (MeasuredMaterials::refractiveIndexFromAbbe(
					glass->refractiveIndexD,
					glass->abbeNumber,
					builder.iorWavelengthNanometers
				));
			}
			if (builder.hasIorWavelength)
			{
				throw std::runtime_error("ior_wavelength requires Sellmeier or Abbe dispersion.");
			}
			return (glass->refractiveIndexD);
		}
		if (builder.hasIorWavelength)
		{
			throw std::runtime_error("ior_wavelength requires a glass preset, Abbe number, or Sellmeier coefficients.");
		}
		return (builder.refractiveIndex);
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

		std::shared_ptr<Dielectric> material = std::make_shared<Dielectric>(
			builder.color,
			materialRefractiveIndex(builder, "dielectric"),
			builder.roughness
		);
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

	SubsurfaceMethod	parseSubsurfaceMethod(const std::string& value)
	{
		const std::string method = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(value));

		if (
			method == "burley"
			|| method == "diffusion"
			|| method == "normalized_diffusion"
			|| method == "normalizeddiffusion"
		)
		{
			return (SUBSURFACE_BURLEY);
		}
		if (method == "thin" || method == "thin_translucent" || method == "thintranslucent")
		{
			return (SUBSURFACE_THIN);
		}
		throw std::runtime_error("Unknown subsurface method: " + value);
	}

	void	parseMaterialProperty(
		MaterialBuilder& builder,
		const std::string& line,
		const SceneFile::internal::SceneFileContext& context
	)
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
		else if (key == "preset" || key == "material_preset" || key == "materialpreset")
		{
			builder.materialPreset = value;
		}
		else if (key == "color" || key == "basecolor" || key == "base_color")
		{
			builder.color = SceneFile::internal::_parseColorValue(value, key, context);
		}
		else if (key == "emission" || key == "emissioncolor" || key == "emission_color")
		{
			builder.emissionColor = SceneFile::internal::_parseColorValue(value, key, context);
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
			builder.conductorEta = SceneFile::internal::_parseColorValue(value, key, context);
		}
		else if (
			key == "metal_preset"
			|| key == "metalpreset"
			|| key == "conductor"
			|| key == "conductor_preset"
			|| key == "conductorpreset"
		)
		{
			builder.conductorPreset = value;
		}
		else if (
			key == "k"
			|| key == "extinction"
			|| key == "extinctioncoefficient"
			|| key == "extinction_coefficient"
			|| key == "conductor_k"
		)
		{
			builder.conductorExtinction = SceneFile::internal::_parseColorValue(value, key, context);
		}
		else if (key == "transmission")
		{
			builder.transmission = std::stod(value);
		}
		else if (key == "clearcoat" || key == "clear_coat" || key == "coat")
		{
			builder.clearcoat = std::stod(value);
		}
		else if (
			key == "clearcoatroughness"
			|| key == "clearcoat_roughness"
			|| key == "clear_coat_roughness"
			|| key == "coatroughness"
			|| key == "coat_roughness"
		)
		{
			builder.clearcoatRoughness = std::stod(value);
		}
		else if (key == "sheen")
		{
			builder.sheen = std::stod(value);
		}
		else if (
			key == "subsurface"
			|| key == "subsurface_weight"
			|| key == "subsurfaceweight"
			|| key == "sss"
		)
		{
			builder.subsurface = std::stod(value);
			builder.hasSubsurface = true;
		}
		else if (
			key == "subsurface_radius"
			|| key == "subsurfaceradius"
			|| key == "sss_radius"
			|| key == "sssradius"
		)
		{
			builder.subsurfaceRadius = SceneFile::internal::_parseColorValue(value, key, context);
			builder.hasSubsurfaceRadius = true;
		}
		else if (
			key == "subsurface_scale"
			|| key == "subsurfacescale"
			|| key == "sss_scale"
			|| key == "sssscale"
		)
		{
			builder.subsurfaceScale = std::stod(value);
			builder.hasSubsurfaceScale = true;
		}
		else if (
			key == "subsurface_color"
			|| key == "subsurfacecolor"
			|| key == "sss_color"
			|| key == "ssscolor"
		)
		{
			builder.subsurfaceColor = SceneFile::internal::_parseColorValue(value, key, context);
			builder.hasSubsurfaceColor = true;
		}
		else if (
			key == "subsurface_method"
			|| key == "subsurfacemethod"
			|| key == "sss_method"
			|| key == "sssmethod"
		)
		{
			builder.subsurfaceMethod = parseSubsurfaceMethod(value);
			builder.hasSubsurfaceMethod = true;
		}
		else if (
			key == "subsurface_profile"
			|| key == "subsurfaceprofile"
			|| key == "sss_profile"
			|| key == "sssprofile"
		)
		{
			builder.subsurfaceProfile = SceneFile::internal::_lowerCopy(SceneFile::internal::_trim(value));
		}
		else if (key == "glossy_color" || key == "glossycolor" || key == "specular_color" || key == "specularcolor")
		{
			builder.glossyColor = SceneFile::internal::_parseColorValue(value, key, context);
			builder.hasGlossyColor = true;
		}
		else if (
			key == "glossy_weight"
			|| key == "glossyweight"
			|| key == "glossy_mix"
			|| key == "glossymix"
		)
		{
			builder.glossyWeight = std::stod(value);
		}
		else if (key == "ior" || key == "refractiveindex" || key == "refractive_index")
		{
			builder.refractiveIndex = std::stod(value);
			builder.hasRefractiveIndex = true;
		}
		else if (
			key == "glass"
			|| key == "glass_preset"
			|| key == "glasspreset"
			|| key == "dielectric_preset"
			|| key == "dielectricpreset"
		)
		{
			builder.glassPreset = value;
		}
		else if (key == "abbe" || key == "abbe_number" || key == "abbenumber" || key == "vd")
		{
			builder.abbeNumber = std::stod(value);
		}
		else if (key == "sellmeier_b" || key == "sellmeierb")
		{
			builder.sellmeierB = SceneFile::internal::_parseVector3Value(value, key);
		}
		else if (key == "sellmeier_c" || key == "sellmeierc")
		{
			builder.sellmeierC = SceneFile::internal::_parseVector3Value(value, key);
		}
		else if (
			key == "ior_wavelength"
			|| key == "iorwavelength"
			|| key == "refractive_index_wavelength"
			|| key == "refractiveindexwavelength"
		)
		{
			builder.iorWavelengthNanometers = parseNanometers(value, "IOR wavelength");
			builder.hasIorWavelength = true;
		}
		else if (
			key == "absorption"
			|| key == "absorptioncoefficient"
			|| key == "absorption_coefficient"
			|| key == "sigma_a"
			|| key == "sigmaa"
		)
		{
			builder.absorptionCoefficient = SceneFile::internal::_parseColorValue(value, key, context);
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
			builder.transmittance = SceneFile::internal::_parseColorValue(value, key, context);
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
				Texture::loadPPM(
					SceneFile::internal::_resolveAssetPath(baseDirectory, builder.texturePath),
					TextureColorRole::ColorSRGB
				)
			));
		}
	}

	std::shared_ptr<Metal>	buildMetalMaterial(const MaterialBuilder& builder)
	{
		const std::string preset = !builder.conductorPreset.empty()
			? builder.conductorPreset
			: builder.materialPreset;
		if (!preset.empty())
		{
			if (builder.conductorEta || builder.conductorExtinction)
			{
				throw std::runtime_error("Metal material defines both conductor preset and explicit eta/k.");
			}
			const MeasuredMaterials::Conductor conductor = MeasuredMaterials::conductorPreset(preset);
			return (std::make_shared<Metal>(
				conductor.eta,
				conductor.extinctionCoefficient,
				builder.fuzz >= 0.0 ? builder.fuzz : builder.roughness
			));
		}
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

	std::shared_ptr<Principled>	buildPrincipledMaterial(const MaterialBuilder& builder)
	{
		if (builder.conductorEta || builder.conductorExtinction)
		{
			throw std::runtime_error("Principled conductor metals use metallic color; use type=metal for eta/k.");
		}
		if (builder.absorptionCoefficient && builder.transmittance)
		{
			throw std::runtime_error("Principled material defines both absorption and transmittance.");
		}
		if (builder.hasTransmittanceDistance && !builder.transmittance)
		{
			throw std::runtime_error("Principled attenuation distance requires transmittance.");
		}

		const double transmission = std::max(builder.transmission, 1.0 - builder.alpha);
		std::shared_ptr<Principled> material = std::make_shared<Principled>(
			builder.color,
			builder.metallic,
			builder.roughness,
			transmission,
			materialRefractiveIndex(builder, "principled"),
			builder.clearcoat,
			builder.clearcoatRoughness,
			builder.sheen
		);
		if (builder.absorptionCoefficient)
		{
			material->setAbsorptionCoefficient(*builder.absorptionCoefficient);
		}
		if (builder.transmittance)
		{
			material->setTransmittance(*builder.transmittance, builder.transmittanceDistance);
		}
		if (!builder.subsurfaceProfile.empty())
		{
			if (builder.subsurfaceProfile == "skin")
			{
				material->setSkinSubsurfaceProfile();
			}
			else
			{
				throw std::runtime_error("Unknown subsurface profile: " + builder.subsurfaceProfile);
			}
		}
		if (builder.hasSubsurfaceMethod)
		{
			material->setSubsurfaceMethod(builder.subsurfaceMethod);
		}
		if (builder.hasSubsurfaceRadius)
		{
			material->setSubsurfaceRadius(builder.subsurfaceRadius);
		}
		if (builder.hasSubsurfaceScale)
		{
			material->setSubsurfaceScale(builder.subsurfaceScale);
		}
		if (builder.hasSubsurfaceColor)
		{
			material->setSubsurfaceColor(builder.subsurfaceColor);
		}
		if (builder.hasSubsurface)
		{
			material->setSubsurface(builder.subsurface);
		}
		return (material);
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
		if (
			!builder.materialPreset.empty()
			&& type != "metal"
			&& type != "dielectric"
			&& type != "principled"
		)
		{
			throw std::runtime_error("Material preset is only valid for metal, dielectric, or principled materials.");
		}
		if (!builder.conductorPreset.empty() && type != "metal")
		{
			throw std::runtime_error("Conductor preset is only valid for metal materials.");
		}
		if (
			(
				!builder.glassPreset.empty()
				|| builder.abbeNumber
				|| builder.sellmeierB
				|| builder.sellmeierC
				|| builder.hasIorWavelength
			)
			&& type != "dielectric"
			&& type != "principled"
		)
		{
			throw std::runtime_error("Glass dispersion properties are only valid for dielectric or principled materials.");
		}
		if (
			(
				builder.hasSubsurface
				|| builder.hasSubsurfaceRadius
				|| builder.hasSubsurfaceScale
				|| builder.hasSubsurfaceColor
				|| builder.hasSubsurfaceMethod
				|| !builder.subsurfaceProfile.empty()
			)
			&& type != "principled"
		)
		{
			throw std::runtime_error("Subsurface properties are only valid for principled materials.");
		}
		if (type == "principled")
		{
			std::shared_ptr<Material> material = buildPrincipledMaterial(builder);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "glossy")
		{
			std::shared_ptr<Material> material = std::make_shared<Glossy>(builder.color, builder.roughness);
			attachTexture(material, builder, baseDirectory);
			return (material);
		}
		if (type == "diffuse_glossy" || type == "diffuseglossy")
		{
			std::shared_ptr<Material> material = std::make_shared<DiffuseGlossy>(
				builder.color,
				builder.hasGlossyColor ? builder.glossyColor : builder.color,
				builder.glossyWeight,
				builder.roughness
			);
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
		const SceneFile::internal::SceneFileContext& context
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
				return (buildMaterial(builder, blockDescription, context.baseDirectory));
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

			parseMaterialProperty(builder, trimmedLine, context);
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
			context
		);
	} while (!stream.eof());
}
