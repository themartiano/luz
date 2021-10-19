#include "Scene.hpp"
#include "Defaults.hpp"
#include "Color.hpp"
#include <limits>

Scene::Scene(void)
{
	this->_thread_count = 5;
	this->_thread_counter = 0;
	this->_x_resolution = D_WIDTH;
	this->_y_resolution = D_HEIGHT;
	this->_rendered_rows = 0;
	this->_epsilon = 0.001f;
	this->_t_max = std::numeric_limits<float>::max();
	this->_sample_count = D_SAMPLE_COUNT;
	this->_max_light_bounces = D_MAX_LIGHT_BOUNCES;
	this->_should_calculate_light = true;
	this->_activeCamera = 0;

	this->_pixelArray = new unsigned char[D_WIDTH * D_HEIGHT * 3];
}

void	Scene::addCamera(Camera camera)
{
	this->_cameras.push_back(camera);
}

void	Scene::addLight(Light light)
{
	this->_lights.push_back(light);
}

void	Scene::addSphere(Sphere sphere)
{
	this->_spheres.push_back(sphere);
}

void	Scene::setXResolution(const short xRes)
{
	this->_x_resolution = xRes;
}

int		Scene::getXResolution(void) const
{
	return (this->_x_resolution);
}

void	Scene::setYResolution(const short yRes)
{
	this->_y_resolution = yRes;
}

int		Scene::getYResolution(void) const
{
	return (this->_y_resolution);
}

// Sets the color for the pixel at 'index', which is a simple X/Y index.
void	Scene::setPixelArray(int index, Color pixelColor)
{
	// Check if index is in range. (x * y * RGB for each pixel)
	if (index < this->_x_resolution * this->_y_resolution * 3)
	{
		this->_pixelArray[(index * 3) + 2] = (unsigned char)(pixelColor.getRed() * 255.0f);
		this->_pixelArray[(index * 3) + 1] = (unsigned char)(pixelColor.getGreen() * 255.0f);
		this->_pixelArray[index * 3] = (unsigned char)(pixelColor.getBlue() * 255.0f);
	}
}

unsigned char*	Scene::getPixelArray(void) const
{
	return (this->_pixelArray);
}

std::vector<Sphere>	Scene::getSpheres(void) const
{
	return (this->_spheres);
}

Camera	Scene::getActiveCamera(void) const
{
	return (this->_cameras[this->_activeCamera]);
}
