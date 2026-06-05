#include "Scene/Scene.hpp"
#include "Defaults.hpp"
#include "Color.hpp"
#include "Utilities.hpp"
#include "ANSIColors.hpp"
#include "ImageFiles/BMP.hpp"
#include "ImageFiles/TIFF.hpp"
#include "Hittables/BVHNode.hpp"
#include "Materials/Emissive.hpp"
#include <limits>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <cmath>

/*
	Constructors & Destructor
*/

// Constructs the Scene with default values
Scene::Scene(void)
{
	this->_image = std::make_unique<Image>(D_WIDTH, D_HEIGHT);

	this->_sampleCount = D_SAMPLE_COUNT;
	this->_adaptiveSampling = false;
	this->_adaptiveMinSamples = D_ADAPTIVE_MIN_SAMPLES;
	this->_adaptiveCheckInterval = D_ADAPTIVE_CHECK_INTERVAL;
	this->_adaptiveThreshold = D_ADAPTIVE_THRESHOLD;
	this->_maxLightBounces = D_MAX_LIGHT_BOUNCES;
	this->_gammaCorrected = true;
	this->_toneMapped = true;
	this->_skyline = 0.5;
	this->_renderSky = SKY_ATMOSPHERE;
	this->_distanceBlueness = true;
	this->_atmosphere = Atmosphere();
	this->_backgroundColor = Color(0.0, 0.0, 0.0);

	this->_defaultRenderOutputFileName = D_RENDER_FILE_NAME;

	this->_activeCamera = 0;

	this->_t_max = std::numeric_limits<double>::max();

	this->_storePixelRenderTimes = false;
	this->_isFromFile = false;
	this->_benchmarkMode = false;
	this->_renderingThreads = CORE_COUNT;
	this->_bloom = true;
	this->_denoise = false;
	this->_denoiseOutputFileName = "";
}

// Properly frees all allocated memory (destructor)
Scene::~Scene(void)
{
}

// Appends 'camera' to the camera vector (list)
void	Scene::addCamera(Camera camera)
{
	this->_cameras.push_back(camera);
}

// Appends 'hittable' to the Hittable vector (list)
void	Scene::addHittable(std::shared_ptr<Hittable> hittable)
{
	this->_hittables.push_back(hittable);
}

// Returns the current Sample Count (rays per pixel)
int	Scene::getSampleCount(void) const
{
	return (this->_sampleCount);
}

// Sets the Sample Count
void	Scene::setSampleCount(const int sampleCount)
{
	if (sampleCount <= 0)
	{
		throw std::invalid_argument("Sample count must be positive.");
	}
	this->_sampleCount = sampleCount;
}

bool	Scene::getAdaptiveSampling(void) const
{
	return (this->_adaptiveSampling);
}

void	Scene::setAdaptiveSampling(bool adaptiveSampling)
{
	this->_adaptiveSampling = adaptiveSampling;
}

int	Scene::getAdaptiveMinSamples(void) const
{
	return (this->_adaptiveMinSamples);
}

void	Scene::setAdaptiveMinSamples(int adaptiveMinSamples)
{
	if (adaptiveMinSamples <= 0)
	{
		throw std::invalid_argument("Adaptive minimum samples must be positive.");
	}
	this->_adaptiveMinSamples = adaptiveMinSamples;
}

int	Scene::getAdaptiveCheckInterval(void) const
{
	return (this->_adaptiveCheckInterval);
}

void	Scene::setAdaptiveCheckInterval(int adaptiveCheckInterval)
{
	if (adaptiveCheckInterval <= 0)
	{
		throw std::invalid_argument("Adaptive check interval must be positive.");
	}
	this->_adaptiveCheckInterval = adaptiveCheckInterval;
}

double	Scene::getAdaptiveThreshold(void) const
{
	return (this->_adaptiveThreshold);
}

void	Scene::setAdaptiveThreshold(double adaptiveThreshold)
{
	if (!std::isfinite(adaptiveThreshold) || adaptiveThreshold <= 0.0)
	{
		throw std::invalid_argument("Adaptive threshold must be positive.");
	}
	this->_adaptiveThreshold = adaptiveThreshold;
}

// Returns the current Maximum Bounces of Light
int		Scene::getMaxLightBounces(void) const
{
	return (this->_maxLightBounces);
}

// Sets the Maximum Bounces of Light
void	Scene::setMaxLightBounces(const int maxLightBounces)
{
	if (maxLightBounces < 0)
	{
		throw std::invalid_argument("Maximum light bounces must be non-negative.");
	}
	this->_maxLightBounces = maxLightBounces;
}

// Returns Gamma Correction (whether or not the render should be gamma corrected)
bool	Scene::getGammaCorrected(void) const
{
	return (this->_gammaCorrected);
}

// Sets Gamma Correction (whether or not the render should be gamma corrected)
void	Scene::setGammaCorrected(bool gammaCorrected)
{
	this->_gammaCorrected = gammaCorrected;
}

bool	Scene::getToneMapped(void) const
{
	return (this->_toneMapped);
}

void	Scene::setToneMapped(bool toneMapped)
{
	this->_toneMapped = toneMapped;
}

// Returns Skyline (used on sky colors interpolation)
double	Scene::getSkyline(void) const
{
	return (this->_skyline);
}

// Returns the RenderSky option
SkyTypes	Scene::getRenderSky(void) const
{
	return (this->_renderSky);
}

// Sets the RenderSky option
void	Scene::setRenderSky(SkyTypes renderSky)
{
	this->_renderSky = renderSky;
}

bool	Scene::getDistanceBlueness(void) const
{
	return (this->_distanceBlueness);
}

void	Scene::setDistanceBlueness(bool distanceBlueness)
{
	this->_distanceBlueness = distanceBlueness;
}

// Sets the Atmosphere object
void	Scene::setAtmosphere(Atmosphere atmosphere)
{
	this->_atmosphere = atmosphere;
}

// Returns the Atmosphere object
Atmosphere	Scene::getAtmosphere(void) const
{
	return (this->_atmosphere);
}

// Returns the Background Color
Color	Scene::getBackgroundColor(void) const
{
	return (this->_backgroundColor);
}

// Sets the Background Color
void	Scene::setBackgroundColor(Color backgroundColor)
{
	this->_backgroundColor = backgroundColor;
}

// Returns the vector (list) of Hittables
const std::vector<std::shared_ptr<Hittable>>&	Scene::getHittables(void) const
{
	return (this->_hittables);
}

// Returns the currently active camera
Camera	Scene::getActiveCamera(void) const
{
	if (this->_cameras.size() > 0)
	{
		return (this->_cameras[this->_activeCamera]);
	}

	return (Camera()); // No camera should be returned
}

// Returns 'true' if this Scene has at least one Camera
bool	Scene::hasCamera(void) const
{
	return (this->_cameras.size() > 0);
}

// Returns the Output File Name
std::string	Scene::getDefaultRenderOutputFileName(void) const
{
	return (this->_defaultRenderOutputFileName);
}

// Sets the Output File Name
void	Scene::setDefaultRenderOutputFileName(std::string defaultRenderOutputFileName)
{
	this->_defaultRenderOutputFileName = defaultRenderOutputFileName;
}

void	Scene::updateLights(void)
{
	std::vector<std::shared_ptr<Hittable>> lights;

	for (std::shared_ptr<Hittable> hittable : this->_hittables)
	{
		if (hittable->getMaterial()->getType() == EMISSIVE)
		{
			lights.push_back(hittable);
		}
	}

	this->_lights = lights;
}

const std::vector<std::shared_ptr<Hittable>>&	Scene::getLights(void) const
{
	return (this->_lights);
}

void	Scene::updateAccelerationStructure(void)
{
	std::vector<std::shared_ptr<Hittable>> boundedHittables;

	this->_unacceleratedHittables.clear();
	boundedHittables.reserve(this->_hittables.size());

	for (const std::shared_ptr<Hittable>& hittable : this->_hittables)
	{
		AABB boundingBox;

		if (hittable->createBoundingBox(boundingBox))
		{
			boundedHittables.push_back(hittable);
		}
		else
		{
			this->_unacceleratedHittables.push_back(hittable);
		}
	}

	if (boundedHittables.empty())
	{
		this->_accelerationStructure = nullptr;
	}
	else if (boundedHittables.size() == 1)
	{
		this->_accelerationStructure = boundedHittables[0];
	}
	else
	{
		this->_accelerationStructure = std::make_shared<BVHNode>(boundedHittables);
	}
}

const std::shared_ptr<Hittable>&	Scene::getAccelerationStructure(void) const
{
	return (this->_accelerationStructure);
}

const std::vector<std::shared_ptr<Hittable>>&	Scene::getUnacceleratedHittables(void) const
{
	return (this->_unacceleratedHittables);
}

bool	Scene::getStorePixelRenderTimes(void) const
{
	return (this->_storePixelRenderTimes);
}

// Sets storePixelRenderTime
void	Scene::setStorePixelRenderTimes(bool storePixelRenderTimes)
{
	this->_storePixelRenderTimes = storePixelRenderTimes;

	if (this->_storePixelRenderTimes)
	{
		this->_pixelRenderTimes.assign(this->_image->getWidth() * this->_image->getHeight(), 0.0);
	}
	else
	{
		this->_pixelRenderTimes.clear();
	}
}

void	Scene::setPixelRenderTime(std::size_t x, std::size_t y, double renderTime)
{
	if (this->_storePixelRenderTimes)
	{
		const std::size_t totalPixels = this->_image->getWidth() * this->_image->getHeight();
		if (this->_pixelRenderTimes.size() != totalPixels)
		{
			this->_pixelRenderTimes.assign(totalPixels, 0.0);
		}
		std::size_t index = (y * this->_image->getWidth()) + x;

		this->_pixelRenderTimes[index] = renderTime;
	}
}

std::unique_ptr<Image>&	Scene::getImage(void)
{
	return (this->_image);
}

std::unique_ptr<Image>	Scene::generateRenderTimeImage(void) const
{
	if (!this->_storePixelRenderTimes)
	{
		return (std::make_unique<Image>(0, 0));
	}


	auto image = std::make_unique<Image>(this->_image->getWidth(), this->_image->getHeight());
	image->initialize();

	if (this->_pixelRenderTimes.empty())
	{
		return (image);
	}

	const double fastest = *std::min_element(this->_pixelRenderTimes.begin(), this->_pixelRenderTimes.end());
	const double slowest = *std::max_element(this->_pixelRenderTimes.begin(), this->_pixelRenderTimes.end());
	const double range = slowest - fastest;

	for (std::size_t i = 0; i < this->_image->getWidth() * this->_image->getHeight(); i++)
	{
		Color orange(1.0, 0.94, 0.84); // Fastest
		Color purple(0.1, 0.0, 0.18); // Slowest

		const double ratio = range > 0.0 ? (this->_pixelRenderTimes[i] - fastest) / range : 0.0;

		image->setPixel(i % this->_image->getWidth(), i / this->_image->getWidth(), (purple - orange) * ratio + orange);
	}

	return (image);
}

void	Scene::setIsFromFile(bool isFromFile)
{
	this->_isFromFile = isFromFile;
}

bool	Scene::getIsFromFile(void) const
{
	return (this->_isFromFile);
}

void	Scene::setBenchmarkMode(bool benchmarkMode)
{
	this->_benchmarkMode = benchmarkMode;
}

bool	Scene::getBenchmarkMode(void) const
{
	return (this->_benchmarkMode);
}

void	Scene::setRenderingThreads(std::size_t renderingThreads)
{
	if (renderingThreads == 0)
	{
		throw std::invalid_argument("Rendering thread count must be positive.");
	}
	this->_renderingThreads = renderingThreads;
}

std::size_t	Scene::getRenderingThreads(void) const
{
	return (this->_renderingThreads);
}

void	Scene::setBloom(bool bloom)
{
	this->_bloom = bloom;
}

bool	Scene::getBloom(void) const
{
	return (this->_bloom);
}

void	Scene::setDenoise(bool denoise)
{
	this->_denoise = denoise;
}

bool	Scene::getDenoise(void) const
{
	return (this->_denoise);
}

void	Scene::setDenoisedImage(std::unique_ptr<Image> denoisedImage)
{
	this->_denoisedImage = std::move(denoisedImage);
}

std::unique_ptr<Image>&	Scene::getDenoisedImage(void)
{
	return (this->_denoisedImage);
}

const std::unique_ptr<Image>&	Scene::getDenoisedImage(void) const
{
	return (this->_denoisedImage);
}

bool	Scene::hasDenoisedImage(void) const
{
	return (this->_denoisedImage != nullptr);
}

void	Scene::clearDenoisedImage(void)
{
	this->_denoisedImage = nullptr;
}

void	Scene::setDenoiseOutputFileName(std::string denoiseOutputFileName)
{
	this->_denoiseOutputFileName = denoiseOutputFileName;
}

std::string	Scene::getDenoiseOutputFileName(void) const
{
	return (this->_denoiseOutputFileName);
}

void	Scene::initializeDenoiseBuffers(std::size_t width, std::size_t height)
{
	this->_denoiseBuffers = std::make_unique<Denoise::NFORBuffers>();
	this->_denoiseBuffers->initialize(width, height);
}

Denoise::NFORBuffers*	Scene::getDenoiseBuffers(void)
{
	return (this->_denoiseBuffers.get());
}

const Denoise::NFORBuffers*	Scene::getDenoiseBuffers(void) const
{
	return (this->_denoiseBuffers.get());
}

void	Scene::clearDenoiseBuffers(void)
{
	this->_denoiseBuffers = nullptr;
}
