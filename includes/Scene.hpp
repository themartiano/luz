#ifndef SCENE_HPP
# define SCENE_HPP

#include <vector>
#include "Camera.hpp"
#include "Light.hpp"
#include "MLXImage.hpp"
#include "Objects/Triangle.hpp"
#include "Objects/Square.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/Cylinder.hpp"
#include "Objects/Plane.hpp"

class	Scene
{
	public:
		Scene(void);
		void	setXResolution(const short xRes);
		void	setYResolution(const short yRes);

	private:
		void			*_mlx;
		void			*_window;
		MLXImage		_mlx_image;
		//pthread_mutex_t	_img_mutex;
		//pthread_t		*_threads;
		int				_thread_count;
		int				_thread_counter;
		//pthread_mutex_t	_thread_counter_mutex;
		short				_x_resolution;
		short				_y_resolution;
		int				_rendered_rows;
		//pthread_mutex_t	_row_counter_mutex;
		float			_epsilon;
		float			_t_max;
		int				_sample_count;
		int				_max_light_bounces;
		Light			_ambient_light;
		std::vector<Camera>		_cameras;
		std::vector<Light>		_lights;
		std::vector<Triangle>	_triangles;
		std::vector<Square>		_squares;
		std::vector<Sphere>		_spheres;
		std::vector<Cylinder>	_cylinders;
		std::vector<Plane>		_planes;
		bool			_should_calculate_light;
};

#endif