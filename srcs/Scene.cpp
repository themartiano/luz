#include "Scene.hpp"
#include "Defaults.hpp"
#include "Color.hpp"
#include <limits>

/*
	Constructors
*/

// Constructs the Scene with default values
Scene::Scene(void)
{
	this->_xResolution = D_WIDTH;
	this->_yResolution = D_HEIGHT;
	this->_sampleCount = D_SAMPLE_COUNT;
	this->_maxLightBounces = D_MAX_LIGHT_BOUNCES;
	this->_activeCamera = 0;

	this->_t_max = std::numeric_limits<float>::max();
}

// Appends 'camera' to the camera vector (list)
void	Scene::addCamera(Camera camera)
{
	this->_cameras.push_back(camera);
}

// Appends 'light' to the light vector (list)
void	Scene::addLight(Light light)
{
	this->_lights.push_back(light);
}

// Appends 'sphere' to the sphere vector (list)
void	Scene::addSphere(Sphere sphere)
{
	this->_spheres.push_back(sphere);
}

// Returns the X resolution (width)
int		Scene::getXResolution(void) const
{
	return (this->_xResolution);
}

// Sets the X resolution (width)
void	Scene::setXResolution(const int xRes)
{
	this->_xResolution = xRes;
}

// Returns the Y resolution (height)
int		Scene::getYResolution(void) const
{
	return (this->_yResolution);
}

// Sets the Y resolution (height)
void	Scene::setYResolution(const int yRes)
{
	this->_yResolution = yRes;
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

// Sets the color for the pixel at 'index', which is a simple X/Y index.
void	Scene::setPixelArray(int index, Color pixelColor)
{
	// Check if index is in range. (x res * y res * RGB for each pixel)
	if (index < this->_xResolution * this->_yResolution * 3)
	{
		this->_pixelArray[(index * 3) + 2] = (unsigned char)(pixelColor.getRed() * 255.0f);
		this->_pixelArray[(index * 3) + 1] = (unsigned char)(pixelColor.getGreen() * 255.0f);
		this->_pixelArray[index * 3] = (unsigned char)(pixelColor.getBlue() * 255.0f);
	}
}

// Returns the pixel array
unsigned char*	Scene::getPixelArray(void) const
{
	return (this->_pixelArray);
}

// Initializes the pixel array using the Scene's width and height (X & Y pixel resolution)
void	Scene::initializePixelArray(void)
{
	this->_pixelArray = new unsigned char[this->_xResolution * this->_yResolution * 3];
}

// Returns the vector (list) of spheres
std::vector<Sphere>	Scene::getSpheres(void) const
{
	return (this->_spheres);
}

// Returns the currently active camera
Camera	Scene::getActiveCamera(void) const
{
	return (this->_cameras[this->_activeCamera]);
}
