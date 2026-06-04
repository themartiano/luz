#pragma once

#include "Camera.hpp"
#include "Hittables/Hittable.hpp"
#include "Atmosphere.hpp"
#include "SkyTypes.hpp"
#include "Image.hpp"
#include <vector>
#include <memory>

class	Scene
{
	public:
		Scene(void);
		~Scene(void);
		void	addCamera(Camera camera);
		void	addHittable(std::shared_ptr<Hittable> hittable);
		int		getSampleCount(void) const;
		void	setSampleCount(const int sampleCount);
		int		getMaxLightBounces(void) const;
		void	setMaxLightBounces(const int maxLightBounces);
		bool	getGammaCorrected(void) const;
		void	setGammaCorrected(bool gamma);
		bool	getToneMapped(void) const;
		void	setToneMapped(bool toneMapped);
		double	getSkyline(void) const;
		SkyTypes	getRenderSky(void) const;
		void	setRenderSky(SkyTypes renderSky);
		bool	getDistanceBlueness(void) const;
		void	setDistanceBlueness(bool distanceBlueness);
		void	setAtmosphere(Atmosphere atmosphere);
		Atmosphere	getAtmosphere(void) const;
		Color	getBackgroundColor(void) const;
		void	setBackgroundColor(Color backgroundColor);
		Camera	getActiveCamera(void) const;
		bool	hasCamera(void) const;
		const std::vector<std::shared_ptr<Hittable>>&	getHittables(void) const;
		std::string	getDefaultRenderOutputFileName(void) const;
		void		setDefaultRenderOutputFileName(std::string defaultRenderOutputFileName);
		void		updateLights(void);
		const std::vector<std::shared_ptr<Hittable>>& getLights(void) const;
		bool		getStorePixelRenderTimes(void) const;
		void		setStorePixelRenderTimes(bool storePixelRenderTime);
		void		setPixelRenderTime(std::size_t x, std::size_t y, double renderTime);
		std::unique_ptr<Image>&	getImage(void);
		std::unique_ptr<Image>	generateRenderTimeImage(void) const;
		void		setIsFromFile(bool isFromFile);
		bool		getIsFromFile(void) const;
		void		setBenchmarkMode(bool benchmarkMode);
		bool		getBenchmarkMode(void) const;
		void		setRenderingThreads(std::size_t renderingThreads);
		std::size_t	getRenderingThreads(void) const;
		void		setBloom(bool bloom);
		bool		getBloom(void) const;


	private:
		double					_t_max;
		int						_sampleCount;
		int						_maxLightBounces;
		bool					_gammaCorrected;
		bool					_toneMapped;
		std::string				_defaultRenderOutputFileName;
		std::unique_ptr<Image>	_image;
		double					_skyline;
		SkyTypes				_renderSky;
		bool					_distanceBlueness;
		Atmosphere				_atmosphere;
		Color					_backgroundColor;
		std::vector<Camera>		_cameras;
		size_t					_activeCamera;
		std::vector<std::shared_ptr<Hittable>>	_hittables;
		std::vector<std::shared_ptr<Hittable>>	_lights;
		bool					_storePixelRenderTimes;
		std::vector<double>		_pixelRenderTimes;
		bool					_isFromFile;
		bool					_benchmarkMode;
		std::size_t				_renderingThreads;
		bool					_bloom;
};
