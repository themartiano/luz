#ifndef SCENE_HPP
# define SCENE_HPP

class	Scene
{
	public:
		Scene(void);

	private:
		void			*_mlx;
		void			*_window;
		t_img			_img;
		pthread_mutex_t	_img_mutex;
		pthread_t		*_threads;
		int				_thread_count;
		int				_thread_counter;
		pthread_mutex_t	_thread_counter_mutex;
		int				_x_res;
		int				_y_res;
		int				_rendered_rows;
		pthread_mutex_t	_row_counter_mutex;
		float			_epsilon;
		float			_t_max;
		int				_samples;
		int				_max_bounces;
		t_light			_amb_light;
		t_object		*_cameras;
		t_object		*_objects;
		t_object		*_lights;
		bool			_should_calculate_light;

};

#endif