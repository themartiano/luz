#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "Light.hpp"
#include "Hittable.hpp"
#include <vector>
#include <memory>

class	Scene
{
	public:
		Scene(void);
		void	addCamera(Camera camera);
		void	addLight(Light light);
		void	addHittable(std::shared_ptr<Hittable> hittable);
		int		getXResolution(void) const;
		void	setXResolution(const int xRes);
		int		getYResolution(void) const;
		void	setYResolution(const int yRes);
		int		getSampleCount(void) const;
		void	setSampleCount(const int sampleCount);
		int		getMaxLightBounces(void) const;
		void	setMaxLightBounces(const int maxLightBounces);
		bool	getGammaCorrected(void) const;
		void	setGammaCorrected(bool gamma);
		float	getSkyline(void) const;
		bool	getRenderSky(void) const;
		void	setRenderSky(bool renderSky);
		void	setPixelArray(int index, Color pixelColor);
		unsigned char*	getPixelArray() const;
		void	initializePixelArray(void);
		Camera	getActiveCamera(void) const;
		std::vector<std::shared_ptr<Hittable>>	getHittables(void) const;

	private:
		int						_xResolution;
		int						_yResolution;
		float					_t_max;
		int						_sampleCount;
		int						_maxLightBounces;
		bool					_gammaCorrected;
		unsigned char*			_pixelArray;
		float					_skyline;
		bool					_renderSky;
		//Light					_ambientLight;
		std::vector<Camera>		_cameras;
		size_t					_activeCamera;
		std::vector<Light>		_lights;
		std::vector<std::shared_ptr<Hittable>>	_hittables;
};

#endif