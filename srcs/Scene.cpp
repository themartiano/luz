#include "Scene.hpp"
#include "Defaults.hpp"
#include "Color.hpp"
#include "Utilities.hpp"
#include <limits>

/*
	Constructors & Destructor
*/

// Constructs the Scene with default values
Scene::Scene(void)
{
	this->_xResolution = D_WIDTH;
	this->_yResolution = D_HEIGHT;
	this->_pixelArray = new unsigned char[this->_xResolution * this->_yResolution * 3];

	this->_sampleCount = D_SAMPLE_COUNT;
	this->_maxLightBounces = D_MAX_LIGHT_BOUNCES;
	this->_gammaCorrected = true;
	this->_skyline = 0.5;
	this->_renderSky = SKY_ATMOSPHERE;
	this->_distanceBlueness = true;
	this->_atmosphere = Atmosphere();
	this->_backgroundColor = Color(0.0, 0.0, 0.0);

	this->_outputFileName = "render.bmp";

	this->_activeCamera = 0;

	this->_t_max = std::numeric_limits<double>::max();
}

// Constructs the Scene with cusotm values for width and height
Scene::Scene(int width, int height)
{
	this->_xResolution = width;
	this->_yResolution = height;
	this->_pixelArray = new unsigned char[this->_xResolution * this->_yResolution * 3];

	this->_sampleCount = D_SAMPLE_COUNT;
	this->_maxLightBounces = D_MAX_LIGHT_BOUNCES;
	this->_gammaCorrected = true;
	this->_skyline = 0.5;
	this->_renderSky = SKY_ATMOSPHERE;
	this->_distanceBlueness = true;
	this->_atmosphere = Atmosphere();
	this->_backgroundColor = Color(0.0, 0.0, 0.0);

	this->_activeCamera = 0;

	this->_t_max = std::numeric_limits<double>::max();
}

// Properly frees all allocated memory (destructor)
Scene::~Scene(void)
{
	delete[] this->_pixelArray;
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

// Returns the X resolution (width)
int		Scene::getXResolution(void) const
{
	return (this->_xResolution);
}

// Sets the X resolution (width)
void	Scene::setXResolution(int width)
{
	this->_xResolution = width;

	delete[] this->_pixelArray;
	this->_pixelArray = new unsigned char[this->_xResolution * this->_yResolution * 3];
}

// Returns the Y resolution (height)
int		Scene::getYResolution(void) const
{
	return (this->_yResolution);
}

// Sets the Y resolution (height)
void	Scene::setYResolution(int height)
{
	this->_yResolution = height;

	delete[] this->_pixelArray;
	this->_pixelArray = new unsigned char[this->_xResolution * this->_yResolution * 3];
}

// Returns the current Sample Count (rays per pixel)
int		Scene::getSampleCount(void) const
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

// Sets the color for the pixel at 'index', which is a simple X/Y index.
void	Scene::setPixelArray(int index, Color pixelColor)
{
	// Check if index is in range. (x res * y res * RGB for each pixel)
	if (index < this->_xResolution * this->_yResolution * 3)
	{
		double r = pixelColor.getRed();
		Utilities::setDoubleRange(r, 0.0, 1.0);
		double g = pixelColor.getGreen();
		Utilities::setDoubleRange(g, 0.0, 1.0);
		double b = pixelColor.getBlue();
		Utilities::setDoubleRange(b, 0.0, 1.0);

		this->_pixelArray[(index * 3) + 2] = (unsigned char)(r * 255.0);
		this->_pixelArray[(index * 3) + 1] = (unsigned char)(g * 255.0);
		this->_pixelArray[index * 3] = (unsigned char)(b * 255.0);
	}
}

// Returns the pixel array
unsigned char*	Scene::getPixelArray(void) const
{
	return (this->_pixelArray);
}

// Returns the vector (list) of Hittables
std::vector<std::shared_ptr<Hittable>>	Scene::getHittables(void) const
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

	return (Camera());
}

// Returns 'true' if this Scene has at least one Camera
bool	Scene::hasCamera(void) const
{
	return (this->_cameras.size() > 0);
}

// Returns the Output File Name
std::string	Scene::getOutputFileName(void) const
{
	return (this->_outputFileName);
}

// Sets the Output File Name
void	Scene::setOutputFileName(std::string outputFileName)
{
	this->_outputFileName = outputFileName;
}

void	Scene::updateLights(void)
{
	for (std::shared_ptr<Hittable> hittable : this->_hittables)
	{
		if (hittable->getMaterial().getIsEmissive())
		{
			this->_lights.push_back(hittable);
		}
	}
}

std::vector<std::shared_ptr<Hittable>>	Scene::getLights(void) const
{
	return (this->_lights);
}
