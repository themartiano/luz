#include "FlagsParser.hpp"
#include "Random.hpp"
#include "SceneFile/SceneFile.hpp"
#include "Scene/SceneHelpers.hpp"

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

		scene.getImage()->setWidth(100);
		scene.getImage()->setHeight(100);
		scene.getImage()->initialize();
		scene.setSampleCount(100);
		scene.setMaxLightBounces(50);
		scene.setGammaCorrected(true);
		scene.setRenderSky(SKY_NONE);
		scene.setDistanceBlueness(false);
		scene.setBackgroundColor(Color(0.0, 0.0, 0.0));
	}
}
