#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <fstream>
#include <optional>
#include <stdexcept>

namespace
{
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

	double	parsePositiveFiniteDouble(const std::string& value, const std::string& name)
	{
		const double result = std::stod(value);
		if (!std::isfinite(result) || result <= 0.0)
		{
			throw std::runtime_error(name + " must be finite and positive.");
		}
		return (result);
	}

	double	parsePositiveMillimetersAsMeters(const std::string& value, const std::string& name)
	{
		return (parsePositiveFiniteDouble(value, name) / 1000.0);
	}

	bool	parseBinaryBool(const std::string& value, const std::string& name)
	{
		const int result = std::stoi(value);
		if (result != 0 && result != 1)
		{
			throw std::runtime_error(name + " must be 0 or 1.");
		}
		return (result == 1);
	}

	void	addCameraBlock(Scene& scene, std::ifstream& stream, const std::string& cameraName)
	{
		std::string line;
		Camera camera;
		std::optional<double> focalLengthMeters;
		std::optional<double> sensorWidthMeters;
		std::optional<double> sensorHeightMeters;
		std::optional<double> fNumber;
		std::optional<double> apertureDiameterMeters;
		std::optional<bool> pinhole;
		std::optional<double> focusDistanceMeters;
		std::optional<double> shutterSeconds;
		std::optional<double> iso;

		do
		{
			getline(stream, line);
			const std::string blockLine = SceneFile::internal::_trim(line);

			if (blockLine.empty() || blockLine.at(0) == '#')
			{
				continue;
			}
			if (blockLine == "}")
			{
				if (focalLengthMeters)
				{
					camera.setFocalLengthMeters(*focalLengthMeters);
				}
				if (sensorWidthMeters)
				{
					camera.setSensorWidthMeters(*sensorWidthMeters);
				}
				if (sensorHeightMeters)
				{
					camera.setSensorHeightMeters(*sensorHeightMeters);
				}
				if (fNumber && apertureDiameterMeters)
				{
					throw std::runtime_error("Camera block '" + cameraName + "' defines both f_stop and aperture_diameter.");
				}
				if (fNumber)
				{
					camera.setFNumber(*fNumber);
				}
				if (apertureDiameterMeters)
				{
					camera.setApertureDiameterMeters(*apertureDiameterMeters);
				}
				if (pinhole)
				{
					if (*pinhole && apertureDiameterMeters)
					{
						throw std::runtime_error("Camera block '" + cameraName + "' defines pinhole=1 and aperture_diameter.");
					}
					camera.setPinhole(*pinhole);
				}
				if (focusDistanceMeters)
				{
					camera.setFocusDistanceMeters(*focusDistanceMeters);
				}
				if (shutterSeconds || iso)
				{
					if (!shutterSeconds || !iso)
					{
						throw std::runtime_error("Camera block '" + cameraName + "' photographic exposure requires shutter and iso.");
					}
					scene.setPhotographicExposure(camera.getFNumber(), *shutterSeconds, *iso);
				}
				scene.addCamera(camera);
				return;
			}

			std::string key;
			std::string value;
			if (!splitAssignment(blockLine, key, value))
			{
				throw std::runtime_error("Invalid camera property: " + blockLine);
			}

			if (key == "position")
			{
				camera.setPosition(SceneFile::internal::_parseVector3Value(value, key));
			}
			else if (key == "direction")
			{
				camera.setDirection(SceneFile::internal::_parseVector3Value(value, key));
			}
			else if (key == "up" || key == "updirection" || key == "up_direction")
			{
				camera.setUpDirection(SceneFile::internal::_parseVector3Value(value, key));
			}
			else if (key == "focal_length")
			{
				focalLengthMeters = parsePositiveFiniteDouble(value, "Camera focal_length");
			}
			else if (key == "focal_length_mm")
			{
				focalLengthMeters = parsePositiveMillimetersAsMeters(value, "Camera focal_length_mm");
			}
			else if (key == "sensor_width")
			{
				sensorWidthMeters = parsePositiveFiniteDouble(value, "Camera sensor_width");
			}
			else if (key == "sensor_width_mm")
			{
				sensorWidthMeters = parsePositiveMillimetersAsMeters(value, "Camera sensor_width_mm");
			}
			else if (key == "sensor_height")
			{
				sensorHeightMeters = parsePositiveFiniteDouble(value, "Camera sensor_height");
			}
			else if (key == "sensor_height_mm")
			{
				sensorHeightMeters = parsePositiveMillimetersAsMeters(value, "Camera sensor_height_mm");
			}
			else if (key == "f_stop" || key == "fstop" || key == "f_number" || key == "fnumber")
			{
				fNumber = parsePositiveFiniteDouble(value, "Camera f_stop");
			}
			else if (key == "aperture_diameter")
			{
				apertureDiameterMeters = parsePositiveFiniteDouble(value, "Camera aperture_diameter");
			}
			else if (key == "aperture_diameter_mm")
			{
				apertureDiameterMeters = parsePositiveMillimetersAsMeters(value, "Camera aperture_diameter_mm");
			}
			else if (key == "pinhole")
			{
				pinhole = parseBinaryBool(value, "Camera pinhole");
			}
			else if (key == "focus_distance")
			{
				focusDistanceMeters = parsePositiveFiniteDouble(value, "Camera focus_distance");
			}
			else if (key == "shutter" || key == "shutter_seconds" || key == "shutterseconds" || key == "shutter_speed" || key == "shutterspeed")
			{
				shutterSeconds = parsePositiveFiniteDouble(value, "Camera shutter");
			}
			else if (key == "iso")
			{
				iso = parsePositiveFiniteDouble(value, "Camera iso");
			}
			else
			{
				throw std::runtime_error("Unknown camera property: " + blockLine);
			}
		} while (!stream.eof());

		throw std::runtime_error("Camera block '" + cameraName + "' is missing a closing }.");
	}
}

// Parses the [scene] section of a Scene file
void	SceneFile::internal::_readSceneSection(Scene& scene, std::ifstream& stream, SceneFile::internal::SceneFileContext& context)
{
	std::string line;
	do
	{
		getline(stream, line);

		if (line.length() <= 0)
		{
			break;
		}
		if (line.at(0) == '#')
		{
			continue;
		}
		std::string lowerLine = line;
		Utilities::toLower(lowerLine);

		if (lowerLine.rfind("camera=", 0) != std::string::npos)
		{
			throw std::runtime_error("Compact camera lines were removed. Use a named camera block with physical lens properties.");
		}
		else if (lowerLine.rfind("camera ", 0) != std::string::npos)
		{
			std::string cameraName;

			if (!_parseNamedBlockHeader(line, "camera", cameraName))
			{
				throw std::runtime_error("Invalid camera block header: " + line);
			}
			addCameraBlock(scene, stream, cameraName);
		}
		else if (lowerLine.rfind("objects{", 0) != std::string::npos)
		{
			internal::_readObjectsSubSection(scene, stream, context);
		}
		else if (_readSceneObjectOrLightBlock(scene, stream, context, line))
		{
			continue;
		}
		else
		{
			throw std::runtime_error("Unknown scene line: " + line);
		}
	} while (!stream.eof());
}
