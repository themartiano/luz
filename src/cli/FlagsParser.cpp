#include "FlagsParser.hpp"
#include "Random.hpp"
#include "SceneFile/SceneFile.hpp"
#include "Scene/SceneHelpers.hpp"
#include "Defaults.hpp"
#include <algorithm>
#include <unistd.h>
#include <stdexcept>
#include <cstdlib>

FlagsParser::FlagsParser(int argc, char** argv)
{
	this->_args = _stringVec(argv + 1, argv + argc);
}

void	FlagsParser::parse(Scene& scene)
{
	// Function call order is important since it'll determine the flag importance
	this->_parseHelp();
	this->_parseSeed();
	this->_parseFile(scene);
	this->_parseBenchmark(scene);
	this->_parseSamples(scene);
	this->_parseMaxLightBounces(scene);
	this->_parseResolution(scene);
	this->_parseDetach();
	this->_parseThreads(scene);
	this->_parseGamma(scene);
	this->_parseToneMapping(scene);
	this->_parseBloom(scene);
	this->_parseRenderTimes(scene);
}

void	FlagsParser::_parseHelp(void)
{
	auto it = this->_findFlag(_stringVec{"--help", "-h"});
	if (it != this->_args.end())
	{
		std::cout
			<< "Usage: ./Luz [options]\n\n"
			<< "Options:\n"
			<< "  -f, --file PATH             Load a .luz scene file\n"
			<< "  -r, --resolution WxH        Override render resolution\n"
			<< "  -s, --samples N             Override samples per pixel\n"
			<< "  -mlb, --maxLightBounces N   Override maximum light bounces\n"
			<< "  -t, --threads N             Render with N worker threads\n"
			<< "  --seed N                    Seed random sampling\n"
			<< "  --gamma true|false          Toggle gamma correction\n"
			<< "  -tm, --tonemapping true|false  Toggle tone mapping\n"
			<< "  --bloom true|false          Toggle bloom\n"
			<< "  --render-times              Write renderTime.bmp\n"
			<< "  --benchmark                 Run the built-in benchmark scene\n";
		exit(EXIT_SUCCESS);
	}
}

FlagsParser::_iterator	FlagsParser::_findFlag(_stringVec flagVariations)
{
	for (auto flag : flagVariations)
	{
		auto it = std::find(this->_args.begin(), this->_args.end(), flag);
		if (it != this->_args.end())
		{
			return (it);
		}
	}

	return (this->_args.end());
}

FlagsParser::_iterator	FlagsParser::_findFlag(std::string flag)
{
	return (_findFlag(_stringVec{flag}));
}

void	FlagsParser::_parseSeed(void)
{
	auto it = this->_findFlag("--seed");
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--seed requires a value.");
		}
		setRandomSeed(std::stoi(*(it + 1)));
	}
}

void	FlagsParser::_parseFile(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--file", "-f"});
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--file requires a path.");
		}
		std::string filename = *(it + 1);
		SceneFile::read(scene, filename);
	}
}

void	FlagsParser::_parseBenchmark(Scene& scene)
{
	auto it = this->_findFlag("--benchmark");
	if (it != this->_args.end())
	{
		scene.setIsFromFile(true); // We simulate that it read a scene file.
		scene.setBenchmarkMode(true);

		SceneHelpers::benchmark(scene);

		scene.getImage()->setWidth(200);
		scene.getImage()->setHeight(200);
		scene.getImage()->initialize();
		scene.setSampleCount(50);
		scene.setMaxLightBounces(8);
		scene.setGammaCorrected(true);
		scene.setToneMapped(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		scene.setBackgroundColor(Color(0.0, 0.0, 0.0));
	}
}

void	FlagsParser::_parseSamples(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--samples", "-s"});
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--samples requires a value.");
		}
		scene.setSampleCount(std::stoi(*(it + 1)));
	}
}

void	FlagsParser::_parseMaxLightBounces(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--maxLightBounces", "-mlb"});
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--maxLightBounces requires a value.");
		}
		scene.setMaxLightBounces(std::stoi(*(it + 1)));
	}
}

void	FlagsParser::_parseResolution(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--resolution", "-r"});
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--resolution requires a value.");
		}
		std::string res = *(it + 1);
		const std::size_t separator = res.find("x");
		if (separator == std::string::npos || separator == 0 || separator == res.length() - 1)
		{
			throw std::runtime_error("--resolution must use WIDTHxHEIGHT format.");
		}
		long long width = std::stoll(res.substr(0, separator));
		long long height = std::stoll(res.substr(separator + 1));
		if (width <= 0 || height <= 0)
		{
			throw std::runtime_error("--resolution dimensions must be positive.");
		}
		scene.getImage()->setWidth(static_cast<std::size_t>(width));
		scene.getImage()->setHeight(static_cast<std::size_t>(height));
		scene.getImage()->initialize();
	}
}

void	FlagsParser::_parseDetach(void)
{
	auto it = this->_findFlag(_stringVec{"--detach", "--detached", "-d"});
	if (it != this->_args.end())
	{
		#if defined __linux__ || defined __APPLE__
			int pid = fork();
			if (pid < 0)
			{
				std::cerr << "An error occurred while detaching the process." << std::endl;
				exit(EXIT_FAILURE);
			}
			else if (pid > 0) // Exits the original process
			{
				std::cout << "Process detached with success. PID: " << pid << std::endl;
				exit(EXIT_SUCCESS);
			}
			else // Sets the new process as the group owner and closes stdin, stdou and stderr.
			{
				setsid();
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
			}
		#else
			std::cerr << "Detaching is not supported on this platform." << std::endl;
			exit(EXIT_FAILURE);
		#endif
	}
}

void	FlagsParser::_parseThreads(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--threads", "-t"});
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--threads requires a value.");
		}
		int threads = std::stoi(*(it + 1));
		if (threads <= 0)
		{
			throw std::runtime_error("--threads must be positive.");
		}
		scene.setRenderingThreads(static_cast<std::size_t>(threads));
	}
}

void	FlagsParser::_parseGamma(Scene& scene)
{
	auto it = this->_findFlag("--gamma");
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--gamma requires a value.");
		}
		std::string gamma = *(it + 1);
		if (gamma == "true" || gamma == "1")
		{
			scene.setGammaCorrected(true);
		}
		else if (gamma == "false" || gamma == "0")
		{
			scene.setGammaCorrected(false);
		}
		else
		{
			throw std::runtime_error("Invalid value for --gamma. Use true, false, 1, or 0.");
		}
	}
}

void	FlagsParser::_parseToneMapping(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--tonemapping", "-tm"});
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--tonemapping requires a value.");
		}
		std::string toneMapping = *(it + 1);
		if (toneMapping == "true" || toneMapping == "1")
		{
			scene.setToneMapped(true);
		}
		else if (toneMapping == "false" || toneMapping == "0")
		{
			scene.setToneMapped(false);
		}
		else
		{
			throw std::runtime_error("Invalid value for " + *it + ". Use true, false, 1, or 0.");
		}
	}
}

void	FlagsParser::_parseBloom(Scene& scene)
{
	auto it = this->_findFlag("--bloom");
	if (it != this->_args.end())
	{
		if (it + 1 == this->_args.end())
		{
			throw std::runtime_error("--bloom requires a value.");
		}
		std::string bloom = *(it + 1);
		if (bloom == "true" || bloom == "1")
		{
			scene.setBloom(true);
		}
		else if (bloom == "false" || bloom == "0")
		{
			scene.setBloom(false);
		}
		else
		{
			throw std::runtime_error("Invalid value for --bloom. Use true, false, 1, or 0.");
		}
	}
}

void	FlagsParser::_parseRenderTimes(Scene& scene)
{
	auto it = this->_findFlag("--render-times");
	if (it != this->_args.end())
	{
		scene.setStorePixelRenderTimes(true);
	}
}
