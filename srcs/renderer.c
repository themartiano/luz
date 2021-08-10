/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/08/10 09:33:12 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	light_bouncer(t_scene *scene, t_vec2 pxl, t_hit_record *hit_rec)
{
	t_vec3	target;
	int		bounces;

	bounces = 0;
	while (bounces < scene->max_bounces)
	{
		//target = sub(sum(sum(hit_rec->p, hit_rec->normal), random_in_unit()), hit_rec->p);
		target = sum(sum(hit_rec->p, hit_rec->normal), random_in_unit());
		if (!check_ray_hits(scene, gen_ray(scene, pxl, hit_rec->p, target), hit_rec))
		{
			break ;
		}
		bounces++;
	}
}

t_ray	gen_ray(t_scene *scene, t_vec2 pxl, t_vec3 origin, t_vec3 dir)
{
	t_ray	ray;
	t_vec3	view_up;
	t_vec3	w;
	t_vec3	u;
	t_vec3	v;

	view_up = set(0.0f, 1.0f, 0.0f);
	w = normalize(sub(origin, dir));
	u = normalize(cross(view_up, w));
	v = normalize(cross(w, u));
	ray.origin = origin;
	ray.direction.x = -get_camera(scene)->half_width + dir.x + (pxl.x * u.x * get_camera(scene)->half_width * 2.0f);
	ray.direction.y = -get_camera(scene)->half_height + dir.y + (pxl.y * v.y * get_camera(scene)->half_height * 2.0f);
	ray.direction.z = dir.z;
	ray.direction = normalize(ray.direction);
	ray.direction.y = -ray.direction.y;
	return (ray);
}

static void	render_loop(t_scene scene, t_color *hit_color, int x, int y)
{
	t_hit_record	hit_rec;
	int				s;

	s = 0;
	*hit_color = set_color(0, 0, 0);
	while (s < scene.samples)
	{
		hit_rec.color = set_color(0, 0, 0);
		hit_rec.l_brightness = 0.0f;
		get_hit_color(&scene, &hit_rec, x, y);
		*hit_color = set_color(hit_color->r + hit_rec.color.r, hit_color->g + hit_rec.color.g, hit_color->b + hit_rec.color.b);
		s++;
	}
	if (s++ > 0)
		*hit_color = set_color(hit_color->r / s, hit_color->g / s, hit_color->b / s);
}

void	*render(void *vscene)
{
	t_scene	*scene = (t_scene *)vscene;
	t_color	hit_color;
	int		thread_nbr;
	int		x;
	int		y;

	usleep(100);
	pthread_mutex_lock(&scene->thread_counter_mutex);
	scene->thread_counter += 1;
	thread_nbr = scene->thread_counter;
	pthread_mutex_unlock(&scene->thread_counter_mutex);

	int grid_size = sqrt(scene->thread_count - 1);
	int grid_row = ceilf((float)thread_nbr / (float)grid_size) - 1.0f;
	y = grid_row * (scene->y_res / grid_size);
	x = (thread_nbr - 1 - (grid_size * grid_row)) * (scene->x_res / grid_size);
	for (int i = y; i < (scene->y_res / grid_size) + y; i++)
	{
		for (int j = x; j < (scene->x_res / grid_size) + x; j++)
		{
			render_loop(*scene, &hit_color, j, i);
			pthread_mutex_lock(&scene->img_mutex);
			put_pixel(&scene->img, j, i, rgba_to_hex(hit_color));
			pthread_mutex_unlock(&scene->img_mutex);
		}
		pthread_mutex_lock(&scene->pxl_counter_mutex);
		scene->rendered_rows += 1;
		pthread_mutex_unlock(&scene->pxl_counter_mutex);
	}
	return (NULL);
}


int	render_manager(t_scene *scene)
{
	static pthread_attr_t	thread_attr;

	if (scene->threads == NULL)
	{
		scene->threads = malloc((scene->thread_count + 1) * sizeof(pthread_t));
		if (scene->threads == NULL)
			exit_error(scene, "MALLOC failed.");
		scene->threads[scene->thread_count] = (pthread_t) NULL;
		pthread_attr_init(&thread_attr);
		pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&scene->threads[0], &thread_attr, output_manager, scene);
		for (int i = 1; i < scene->thread_count; i++)
		{
			pthread_create(&scene->threads[i], &thread_attr, render, scene);
		}
	}
	if (scene->mlx != NULL && scene->window != NULL)
		mlx_put_image_to_window(scene->mlx, scene->window, scene->img.img, 0, 0);
	return (0);
}
