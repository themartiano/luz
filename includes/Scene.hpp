#pragma once

#include "Camera.hpp"
#include "Hittables/Hittable.hpp"
#include "Atmosphere.hpp"
#include "SkyTypes.hpp"
#include "ImageFiles/Types.hpp"
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
		void	setPixel(int x, int y, Color pixelColor);
		std::vector<double>	getPixels() const;
		Color	getPixel(int x, int y) const;
		Camera	getActiveCamera(void) const;
		bool	hasCamera(void) const;
		std::vector<std::shared_ptr<Hittable> >	getHittables(void) const;
		std::string	getDefaultRenderOutputFileName(void) const;
		void		setDefaultRenderOutputFileName(std::string defaultRenderOutputFileName);
		void		updateLights(void);
		std::vector<std::shared_ptr<Hittable> > getLights(void) const;
		void		saveRenderToFile(ImageFileTypes imageFileType); // Add const qualifier
		void		saveRenderToFile(std::string fileName, ImageFileTypes imageFileType); // Add const qualifier
		bool		getStorePixelRenderTimes(void) const;
		void		setStorePixelRenderTimes(bool storePixelRenderTime);
		void		setPixelRenderTime(int x, int y, double renderTime);
		void		savePixelRenderTimesToFile(std::string fileName, ImageFileTypes imageFileType);
		void		savePixelRenderTimesToFile(ImageFileTypes imageFileType);

	private:
		int						_xResolution;
		int						_yResolution;
		double					_t_max;
		int						_sampleCount;
		int						_maxLightBounces;
		bool					_gammaCorrected;
		std::string				_defaultRenderOutputFileName;
		std::vector<double>		_pixels; // Create 'image' object (also use it on _renderTimePixels)
		double					_skyline;
		SkyTypes				_renderSky;
		bool					_distanceBlueness;
		Atmosphere				_atmosphere;
		Color					_backgroundColor;
		std::vector<Camera>		_cameras;
		size_t					_activeCamera;
		std::vector<std::shared_ptr<Hittable> >	_hittables;
		std::vector<std::shared_ptr<Hittable> >	_lights;
		bool					_storePixelRenderTimes;
		std::vector<double>		_pixelRenderTimes;
		std::vector<double>		_renderTimePixels; // Image
};
