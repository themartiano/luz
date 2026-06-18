#include "SceneFileInternal.hpp"
#include "Utilities.hpp"
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

	void	addCameraBlock(Scene& scene, std::ifstream& stream, const std::string& cameraName)
	{
		std::string line;
		Camera camera;
		std::optional<double> fNumber;
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
				if (fNumber || shutterSeconds || iso)
				{
					if (!fNumber || !shutterSeconds || !iso)
					{
						throw std::runtime_error("Camera block '" + cameraName + "' photographic exposure requires f_stop, shutter, and iso.");
					}
					scene.setPhotographicExposure(*fNumber, *shutterSeconds, *iso);
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
			else if (key == "fov")
			{
				camera.setFOV(std::stod(value));
			}
			else if (key == "aperture")
			{
				camera.setAperture(std::stod(value));
			}
			else if (key == "focusdistance" || key == "focus_distance")
			{
				camera.setFocusDistance(std::stod(value));
			}
			else if (key == "f_stop" || key == "fstop" || key == "f_number" || key == "fnumber")
			{
				fNumber = std::stod(value);
			}
			else if (key == "shutter" || key == "shutter_seconds" || key == "shutterseconds" || key == "shutter_speed" || key == "shutterspeed")
			{
				shutterSeconds = std::stod(value);
			}
			else if (key == "iso")
			{
				iso = std::stod(value);
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
			double pX, pY, pZ, dX, dY, dZ, fov, aperture, focusDistance;

			if (sscanf(lowerLine.c_str(), "camera=(%lf,%lf,%lf),(%lf,%lf,%lf),%lf,%lf,%lf", &pX, &pY, &pZ, &dX, &dY, &dZ, &fov, &aperture, &focusDistance) == 9)
			{
				Camera camera;

				camera.setPosition(Vector3(pX, pY, pZ));
				camera.setDirection(Vector3(dX, dY, dZ));
				camera.setFOV(fov);
				camera.setAperture(aperture);
				camera.setFocusDistance(focusDistance);

				scene.addCamera(camera);
			}
			else
			{
				throw std::runtime_error("Invalid camera line: " + line);
			}
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
