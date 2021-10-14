#include "Scene.hpp"
#include <limits>

Scene::Scene(void)
{
	this->_mlx = nullptr;
	this->_window = nullptr;
	//this->_mlx_image = ;
	//this->_img_mutex = ; //mutex
	//this->_threads = nullptr;
	this->_thread_count = 5;
	this->_thread_counter = 0;
	//this->_thread_counter_mutex = ; //mutex
	this->_x_resolution = 0;
	this->_y_resolution = 0;
	this->_rendered_rows = 0;
	//this->_row_counter_mutex = ; //mutex
	this->_epsilon = 0.001f;
	this->_t_max = std::numeric_limits<float>::max();
	this->_sample_count = 48;
	this->_max_light_bounces = 12;
	//this->_ambient_light = ;
	// this->_cameras = ;
	// this->_lights = ;
	// this->_triangles = ;
	// this->_squares = ;
	// this->_spheres = ;
	// this->_cylinders = ;
	// this->_planes = ;
	this->_should_calculate_light = true;
}

void	Scene::setXResolution(const short xRes)
{
	this->_x_resolution = xRes;
}

void	Scene::setYResolution(const short yRes)
{
	this->_y_resolution = yRes;
}
