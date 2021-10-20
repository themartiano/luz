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
	this->_threadCount = 5;
	this->_threadCounter = 0;
	this->_xResolution = D_WIDTH;
	this->_yResolution = D_HEIGHT;
	this->_renderedRows = 0;
	this->_epsilon = 0.001f;
	this->_t_max = std::numeric_limits<float>::max();
	this->_sampleCount = D_SAMPLE_COUNT;
	this->_maxLightBounces = D_MAX_LIGHT_BOUNCES;
	this->_shouldCalculateLight = true;
	this->_activeCamera = 0;

	this->_pixelArray = new unsigned char[D_WIDTH * D_HEIGHT * 3];
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
void	Scene::setXResolution(const short xRes)
{
	this->_xResolution = xRes;
}

// Returns the Y resolution (height)
int		Scene::getYResolution(void) const
{
	return (this->_yResolution);
}

// Sets the Y resolution (height)
void	Scene::setYResolution(const short yRes)
{
	this->_yResolution = yRes;
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
