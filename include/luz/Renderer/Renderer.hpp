#pragma once

#include "Scene/Scene.hpp"

namespace	Renderer
{
	bool	render(Scene& scene);
	void	renderSequence(Scene& scene, Atmosphere baseAtmosphere, int fps, double duration);
}
