#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Scene.hpp"
#include "Ray.hpp"
#include "Color.hpp"
#include "PDFs/MixturePDF.hpp"

class   Renderer
{
    public:
        static bool	render(Scene& scene);

    private:
        static void		_threadRender(Scene& scene, int x, int y);
        static Color	_calculatePixelColor(Scene& scene, int x, int y);
        static bool		_checkHits(Scene& scene, Ray& ray);
        static Color	_calculateLightRaysColor(Ray& ray, Scene& scene, int bounces);
        static void		_calculateLightRayBounceDirection(Ray& ray, Color& color, const Vector3& pdfGen);
        static Color	_computeAtmosphereColor(Scene& scene, Ray& ray);
        static Color	_calculateSkyInterpolation(Scene& scene, Ray& ray);
        static void     _manageThreads(Scene& scene);
};

#endif