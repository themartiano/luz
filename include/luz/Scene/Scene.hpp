#pragma once

#include "Camera.hpp"
#include "Hittables/Hittable.hpp"
#include "Atmosphere.hpp"
#include "SkyTypes.hpp"
#include "Image.hpp"
#include "Denoise/NFOR.hpp"
#include "OutputFormat.hpp"
#include <cstddef>
#include <vector>
#include <memory>

struct	SceneRenderStats
{
	std::size_t	renderedSamples = 0;
	double		averageSamplesPerPixel = 0.0;
	double		renderMS = 0.0;
	double		denoiseMS = 0.0;
	double		postProcessMS = 0.0;
	double		totalMS = 0.0;
};

class	Scene
{
	public:
		Scene(void);
		~Scene(void);
		void	addCamera(Camera camera);
		void	addHittable(std::shared_ptr<Hittable> hittable);
		int		getSampleCount(void) const;
		void	setSampleCount(const int sampleCount);
		bool	getAdaptiveSampling(void) const;
		void	setAdaptiveSampling(bool adaptiveSampling);
		int		getAdaptiveMinSamples(void) const;
		void	setAdaptiveMinSamples(int adaptiveMinSamples);
		int		getAdaptiveCheckInterval(void) const;
		void	setAdaptiveCheckInterval(int adaptiveCheckInterval);
		double	getAdaptiveThreshold(void) const;
		void	setAdaptiveThreshold(double adaptiveThreshold);
		int		getMaxLightBounces(void) const;
		void	setMaxLightBounces(const int maxLightBounces);
		bool	getGammaCorrected(void) const;
		void	setGammaCorrected(bool gamma);
		bool	getToneMapped(void) const;
		void	setToneMapped(bool toneMapped);
		double	getExposure(void) const;
		void	setExposure(double exposure);
		double	getContrast(void) const;
		void	setContrast(double contrast);
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
		RenderOutputFormat	getRenderOutputFormat(void) const;
		void		setRenderOutputFormat(RenderOutputFormat renderOutputFormat);
		void		updateLights(void);
		const std::vector<std::shared_ptr<Hittable>>& getLights(void) const;
		const std::vector<double>&	getLightSelectionCumulativeWeights(void) const;
		double		getLightSelectionTotalWeight(void) const;
		void		updateAccelerationStructure(void);
		const std::shared_ptr<Hittable>&	getAccelerationStructure(void) const;
		const std::vector<std::shared_ptr<Hittable>>&	getUnacceleratedHittables(void) const;
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
		void		setDenoise(bool denoise);
		bool		getDenoise(void) const;
		void		setDenoisedImage(std::unique_ptr<Image> denoisedImage);
		std::unique_ptr<Image>&	getDenoisedImage(void);
		const std::unique_ptr<Image>&	getDenoisedImage(void) const;
		bool		hasDenoisedImage(void) const;
		void		clearDenoisedImage(void);
		void		setDenoiseOutputFileName(std::string denoiseOutputFileName);
		std::string	getDenoiseOutputFileName(void) const;
		void		initializeDenoiseBuffers(std::size_t width, std::size_t height);
		Denoise::NFORBuffers*	getDenoiseBuffers(void);
		const Denoise::NFORBuffers*	getDenoiseBuffers(void) const;
		void		clearDenoiseBuffers(void);
		void		resetRenderStats(void);
		void		setRenderStats(SceneRenderStats renderStats);
		const SceneRenderStats&	getRenderStats(void) const;


	private:
		double					_t_max;
		int						_sampleCount;
		bool					_adaptiveSampling;
		int						_adaptiveMinSamples;
		int						_adaptiveCheckInterval;
		double					_adaptiveThreshold;
		int						_maxLightBounces;
		bool					_gammaCorrected;
		bool					_toneMapped;
		double					_exposure;
		double					_contrast;
		std::string				_defaultRenderOutputFileName;
		RenderOutputFormat		_renderOutputFormat;
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
		std::vector<double>		_lightSelectionCumulativeWeights;
		double					_lightSelectionTotalWeight;
		std::shared_ptr<Hittable>	_accelerationStructure;
		std::vector<std::shared_ptr<Hittable>>	_unacceleratedHittables;
		bool					_storePixelRenderTimes;
		std::vector<double>		_pixelRenderTimes;
		bool					_isFromFile;
		bool					_benchmarkMode;
		std::size_t				_renderingThreads;
		bool					_bloom;
		bool					_denoise;
		std::unique_ptr<Image>	_denoisedImage;
		std::string				_denoiseOutputFileName;
		std::unique_ptr<Denoise::NFORBuffers>	_denoiseBuffers;
		SceneRenderStats		_renderStats;
};
