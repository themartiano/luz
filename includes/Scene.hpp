#pragma once

#include "Camera.hpp"
#include "Hittable.hpp"
#include "Atmosphere.hpp"
#include "SkyTypes.hpp"
#include <vector>
#include <memory>

class	Scene
{
	public:
		Scene(void);
		Scene(int width, int height);
		~Scene(void);
		void	addCamera(Camera camera);
		void	addHittable(std::shared_ptr<Hittable> hittable);
		int		getXResolution(void) const;
		void	setXResolution(int width);
		int		getYResolution(void) const;
		void	setYResolution(int height);
		int		getSampleCount(void) const;
		void	setSampleCount(const int sampleCount);
		int		getMaxLightBounces(void) const;
		void	setMaxLightBounces(const int maxLightBounces);
		bool	getGammaCorrected(void) const;
		void	setGammaCorrected(bool gamma);
		double	getSkyline(void) const;
		SkyTypes	getRenderSky(void) const;
		void	setRenderSky(SkyTypes renderSky);
		bool	getDistanceBlueness(void) const;
		void	setDistanceBlueness(bool distanceBlueness);
		void	setAtmosphere(Atmosphere atmosphere);
		Atmosphere	getAtmosphere(void) const;
		Color	getBackgroundColor(void) const;
		void	setBackgroundColor(Color backgroundColor);
		void	setPixelArray(int index, Color pixelColor);
		unsigned char*	getPixelArray() const;
		Camera	getActiveCamera(void) const;
		bool	hasCamera(void) const;
		std::vector<std::shared_ptr<Hittable>>	getHittables(void) const;
		std::string	getOutputFileName(void) const;
		void		setOutputFileName(std::string outputFileName);
		void		updateLights(void);
		std::vector<std::shared_ptr<Hittable>> getLights(void) const;

	private:
		int						_xResolution;
		int						_yResolution;
		double					_t_max;
		int						_sampleCount;
		int						_maxLightBounces;
		bool					_gammaCorrected;
		std::string				_outputFileName;
		unsigned char*			_pixelArray;
		double					_skyline;
		SkyTypes				_renderSky;
		bool					_distanceBlueness;
		Atmosphere				_atmosphere;
		Color					_backgroundColor;
		std::vector<Camera>		_cameras;
		size_t					_activeCamera;
		std::vector<std::shared_ptr<Hittable>>	_hittables;
		std::vector<std::shared_ptr<Hittable>>	_lights;
};
