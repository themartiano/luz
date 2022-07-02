#include "FlagsParser.hpp"
#include "Random.hpp"
#include "SceneFile/SceneFile.hpp"
#include "Scene/SceneHelpers.hpp"
#include "Defaults.hpp"
#include <unistd.h>

FlagsParser::FlagsParser(int argc, char** argv)
{
	this->_args = _stringVec(argv + 1, argv + argc);
}

void	FlagsParser::parse(Scene& scene)
{
	// Function call order is important since it'll determine the flag importance
	this->_parseFile(scene);
	this->_parseBenchmark(scene);
	this->_parseSeed();
	this->_parseSamples(scene);
	this->_parseMaxLightBounces(scene);
	this->_parseResolution(scene);
	this->_parseDetach();
	this->_parseThreads(scene);
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
		Random::setSeed(std::stoi(*(it + 1)));
	}
}

void	FlagsParser::_parseFile(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--file", "-f"});
	if (it != this->_args.end())
	{
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

		scene.getImage()->setWidth(1000);
		scene.getImage()->setHeight(1000);
		scene.getImage()->initialize();
		scene.setSampleCount(2048);
		scene.setMaxLightBounces(64);
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
		scene.setSampleCount(std::stoi(*(it + 1)));
	}
}

void	FlagsParser::_parseMaxLightBounces(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--maxLightBounces", "-mlb"});
	if (it != this->_args.end())
	{
		scene.setMaxLightBounces(std::stoi(*(it + 1)));
	}
}

void	FlagsParser::_parseResolution(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--resolution", "-r"});
	if (it != this->_args.end())
	{
		std::string res = *(it + 1);
		scene.getImage()->setWidth(std::stoi(res.substr(0, res.find("x"))));
		scene.getImage()->setHeight(std::stoi(res.substr(res.find("x") + 1)));
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
			std::cerr << "Detaching is not supported on this platform." << std::endl << "Continuing without detaching." << std::endl;
		#endif
	}
}

void	FlagsParser::_parseThreads(Scene& scene)
{
	auto it = this->_findFlag(_stringVec{"--threads", "-t"});
	if (it != this->_args.end())
	{
		int threads = std::stoi(*(it + 1));
		if (threads > 0)
		{
			scene.setRenderingThreads(threads);
		}
		else
		{
			scene.setRenderingThreads(CORE_COUNT);
		}
	}
}
