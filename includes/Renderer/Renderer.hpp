#pragma once

#include "Scene.hpp"
#include "Ray/Ray.hpp"
#include "Color.hpp"
#include "PDFs/MixturePDF.hpp"

namespace	Renderer
{
	bool	render(Scene& scene);
	void	renderSequence(Scene& scene, Atmosphere baseAtmosphere, int fps, double duration, ImageFileTypes imageFileType);

	namespace internal
	{
		void	_threadRender(Scene& scene, int x, int y);
		Color	_calculatePixelColor(Scene& scene, int x, int y);
		bool	_checkHits(Scene& scene, Ray& ray);
		Color	_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
		void	_calculateLightRayBounceDirection(Ray& ray, Color& color, const Vector3& pdfGen);
		Color	_computeAtmosphereColor(Scene& scene, Ray& ray);
		Color	_calculateSkyInterpolation(Scene& scene, Ray& ray);
		void	_manageThreads(Scene& scene);
	}
}
