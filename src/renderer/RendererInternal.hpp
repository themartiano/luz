#pragma once

#include "Color.hpp"
#include "Ray/Ray.hpp"
#include "Scene/Scene.hpp"

namespace Renderer::internal
{
	void	_threadRender(Scene& scene, std::size_t x, std::size_t y);
	Color	_calculatePixelColor(Scene& scene, std::size_t x, std::size_t y);
	bool	_checkHits(Scene& scene, Ray& ray, HitRecord& hitRecord);
	Color	_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
	Color	_computeAtmosphereColor(Scene& scene, Ray& ray);
	Color	_calculateSkyInterpolation(Scene& scene, Ray& ray);
	void	_manageThreads(Scene& scene);
	Ray		_generateRay(Scene& scene, std::size_t x, std::size_t y);
}
