#include "Scene/Scene.hpp"
#include "Defaults.hpp"
#include "Color.hpp"
#include "Utilities.hpp"
#include "ANSIColors.hpp"
#include "ImageFiles/BMP.hpp"
#include "ImageFiles/TIFF.hpp"
#include "Materials/Emissive.hpp"
#include <limits>
#include <utility>

/*
	Constructors & Destructor
*/

// Constructs the Scene with default values
Scene::Scene(void)
{
	this->_image = std::make_unique<Image>(D_WIDTH, D_HEIGHT);

	this->_sampleCount = D_SAMPLE_COUNT;
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
	this->_sampleCount = sampleCount;
}

// Returns the current Maximum Bounces of Light
int		Scene::getMaxLightBounces(void) const
{
	return (this->_maxLightBounces);
}

// Sets the Maximum Bounces of Light
void	Scene::setMaxLightBounces(const int maxLightBounces)
{
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

std::vector<std::shared_ptr<Hittable>>	Scene::getLights(void) const
{
	return (this->_lights);
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
		this->_pixelRenderTimes.reserve(this->_image->getWidth() * this->_image->getHeight());

		for (std::size_t i = 0; i < this->_image->getWidth() * this->_image->getHeight(); i++)
		{
			this->_pixelRenderTimes.push_back(0.0);
		}
	}
}

void	Scene::setPixelRenderTime(std::size_t x, std::size_t y, double renderTime)
{
	if (this->_storePixelRenderTimes)
	{
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

	// Looks like it is not possible to use iterators because we're settings values with [] so the vector doesn't properly recognizes it. size() is 0 btw
	// const double fastest = *std::min_element(this->_pixelRenderTimes.begin(), this->_pixelRenderTimes.end());
	// const double slowest = *std::max_element(this->_pixelRenderTimes.begin(), this->_pixelRenderTimes.end());

	double fastest = 0.0;
	double slowest = 0.0;

	for (std::size_t i = 0; i < this->_image->getWidth() * this->_image->getHeight(); i++)
	{
		if (this->_pixelRenderTimes[i] < fastest)
		{
			fastest = this->_pixelRenderTimes[i];
		}

		if (this->_pixelRenderTimes[i] > slowest)
		{
			slowest = this->_pixelRenderTimes[i];
		}
	}

	for (std::size_t i = 0; i < this->_image->getWidth() * this->_image->getHeight(); i++)
	{
		Color orange(1.0, 0.94, 0.84); // Fastest
		Color purple(0.1, 0.0, 0.18); // Slowest

		// interpolate between orange (fast) and purple (slow)
		const double ratio = (this->_pixelRenderTimes[i] - fastest) / (slowest - fastest);

		// interpolate between two colors using 'ratio'

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
