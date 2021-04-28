/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/28 16:24:21 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	light_bouncer(t_scene *scene, t_vec3 uv, t_hit_record *hit_rec)
{
	t_vec3	tmp_vec3;
	t_vec3	target;
	int		bounces;

	bounces = 0;
	while (bounces < scene->max_bounces)
	{
		tmp_vec3 = sum(sum(hit_rec->p, hit_rec->normal), random_in_unit());
		target = sub(tmp_vec3, hit_rec->p);
		target.x = 0;
		target.y = 0;
		target.z -= 1.0f;
		if (!check_ray_hits(scene, gen_ray(scene, uv,
					hit_rec->p, target), hit_rec))
		{
			break ;
		}
		bounces++;
	}
}

t_ray	gen_ray(t_scene *scene, t_vec3 uv, t_vec3 origin, t_vec3 dir)
{
	t_ray	ray;
	t_vec3	view_up;
	t_vec3	w;
	t_vec3	u;
	t_vec3	v;

	view_up = set(0, 1, 0);
	w = normalize(sub(origin, dir));
	u = normalize(cross(view_up, w));
	v = cross(w, u);
	ray.origin.x = origin.x;
	ray.origin.y = origin.y;
	ray.origin.z = origin.z;
	ray.direction.x = -scene->camera.half_width + dir.x + (uv.x * u.x
			* scene->camera.half_width * 2.0f);
	ray.direction.y = -scene->camera.half_height + dir.y + (uv.y * v.y
			* scene->camera.half_height * 2.0f);
	ray.direction.z = dir.z;
	ray.direction = normalize(ray.direction);
	ray.direction.y = -ray.direction.y;
	return (ray);
}

static void	render_loop(t_scene *scene, t_color *hit_color,
int x, int y)
{
	t_hit_record	hit_rec;
	t_vec3			current_pixel;
	int				s;

	s = 0;
	current_pixel.x = x;
	current_pixel.y = y;
	hit_color->r = 255;
	hit_color->g = 255;
	hit_color->b = 255;
	while (s < scene->samples)
	{
		hit_rec.color = set_color(255, 255, 255);
		get_hit_color(scene, &hit_rec, current_pixel);
		*hit_color = set_color(hit_color->r + hit_rec.color.r,
			hit_color->g + hit_rec.color.g, hit_color->b + hit_rec.color.b);
		s++;
	}
	if (s++ > 0)
		*hit_color = set_color(hit_color->r / s, hit_color->g / s,
			hit_color->b / s);
}

void	*render(void *vscene)
{
	t_scene		*scene;
	t_color		hit_color;
	int			x;
	int			y;

	scene = (t_scene *)vscene;
	y = 0;
	while (y < scene->y_res)
	{
		x = 0;
		while (x < scene->x_res)
		{
			render_loop(scene, &hit_color, x, y);
			put_pixel(&scene->img, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
	printf(COLOR_LIGHT_GREEN "Render done!\n\n" COLOR_NC);
	write_bmp(scene);
	return (NULL);
}

int	render_manager(t_scene *scene)
{
	static pthread_t		thread_id;
	static pthread_attr_t	thread_attr;
	static bool				created = false;

	if (created == false)
	{
		printf(COLOR_YELLOW "Starting rendering thread...\n" COLOR_NC);
		pthread_attr_init(&thread_attr);
		if (scene->window != NULL)
			pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
		printf(COLOR_YELLOW "Rendering...\n" COLOR_NC);
		pthread_create(&thread_id, &thread_attr, render, scene);
		if (scene->window == NULL)
		{
			pthread_join(thread_id, NULL);
			clean_exit(scene);
		}
		created = true;
	}
	if (scene->mlx != NULL && scene->window != NULL)
		mlx_put_image_to_window(scene->mlx, scene->window, scene->img.img,
			0, 0);
	return (0);
}
