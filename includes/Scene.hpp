#ifndef SCENE_HPP
# define SCENE_HPP

#include "Camera.hpp"
#include "Light.hpp"
#include "Objects/Triangle.hpp"
#include "Objects/Square.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/Cylinder.hpp"
#include "Objects/Plane.hpp"
#include <vector>

class	Scene
{
	public:
		Scene(void);
		void	addCamera(Camera camera);
		void	addLight(Light light);
		void	addSphere(Sphere sphere);
		void	setXResolution(const short xRes);
		int		getXResolution(void) const;
		void	setYResolution(const short yRes);
		int		getYResolution(void) const;
		void	setPixelArray(int index, Color pixelColor);
		unsigned char*	getPixelArray() const;
		std::vector<Sphere>	getSpheres(void) const;
		Camera	getActiveCamera(void) const;

	private:
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
		unsigned char*	_pixelArray;
		size_t			_activeCamera;
};

#endif