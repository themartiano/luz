#pragma once

#include "Scene.hpp"
#include "Ray/Ray.hpp"
#include "Color.hpp"
#include "PDFs/MixturePDF.hpp"

namespace	Renderer
{
	bool	render(Scene& scene);
	void	renderSequence(Scene& scene, Atmosphere baseAtmosphere, int fps, double duration);

	namespace internal
	{
		void	_threadRender(Scene& scene, int x, int y);
		Color	_calculatePixelColor(Scene& scene, int x, int y);
		bool	_checkHits(Scene& scene, Ray& ray, HitRecord& hitRecord);
		Color	_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
		Color	_computeAtmosphereColor(Scene& scene, Ray& ray);
		Color	_calculateSkyInterpolation(Scene& scene, Ray& ray);
		void	_manageThreads(Scene& scene);
		Ray		_generateRay(Scene& scene, int x, int y);
	}
}
