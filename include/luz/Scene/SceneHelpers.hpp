#pragma once

#include "Scene/Scene.hpp"
#include <string>

namespace SceneHelpers
{
	void	cornellBox(Scene& scene);
	void	cornellBox(Scene& scene, bool cubes);
	void	benchmark(Scene& scene);
	void	benchmark(Scene& scene, const std::string& benchmarkCase);
}
