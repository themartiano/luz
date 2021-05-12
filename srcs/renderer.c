/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/12 14:36:25 by ejuliao-         ###   ########.fr       */
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
		target = sub(sum(sum(hit_rec->p, hit_rec->normal), random_in_unit()),
				hit_rec->p);
		if (!check_ray_hits(scene, gen_ray(scene, pxl,
					hit_rec->p, target), hit_rec))
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
	ray.origin.x = origin.x;
	ray.origin.y = origin.y;
	ray.origin.z = origin.z;
	ray.direction.x = -get_camera(scene)->half_width + dir.x + (pxl.x * u.x
			* get_camera(scene)->half_width * 2.0f);
	ray.direction.y = -get_camera(scene)->half_height + dir.y + (pxl.y * v.y
			* get_camera(scene)->half_height * 2.0f);
	ray.direction.z = dir.z;
	ray.direction = normalize(ray.direction);
	ray.direction.y = -ray.direction.y;
	return (ray);
}

static void	render_loop(t_scene *scene, t_color *hit_color,
int x, int y)
{
	t_hit_record	hit_rec;
	int				s;

	s = 0;
	*hit_color = set_color(0, 0, 0);
	while (s < scene->samples)
	{
		hit_rec.color = set_color(0, 0, 0);
		hit_rec.l_brightness = 0.0f;
		get_hit_color(scene, &hit_rec, x, y);
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
			scene->crrnt_pxl.x = x;
			scene->crrnt_pxl.y = y;
			render_loop(scene, &hit_color, x, y);
			put_pixel(&scene->img, x, y, rgba_to_hex(hit_color));
			x++;
		}
		sleep(0);
		y++;
	}
	printf(COLOR_LIGHT_GREEN "\nRender done!\n\n" COLOR_NC);
	write_bmp(scene);
	scene->thread = (pthread_t) NULL;
	return (NULL);
}

int	render_manager(t_scene *scene)
{
	static pthread_attr_t	thread_attr;
	static t_camera			*camera = NULL;

	if (scene->thread == (pthread_t) NULL && camera != get_camera(scene))
	{
		camera = get_camera(scene);
		printf(COLOR_YELLOW "Starting rendering thread...\n" COLOR_NC);
		pthread_attr_init(&thread_attr);
		if (scene->window != NULL)
			pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
		printf(COLOR_YELLOW "Rendering..." COLOR_YELLOW " (" COLOR_WHITE "%d "
			COLOR_CYAN"samples, "COLOR_WHITE"%d"COLOR_CYAN" max light bounces"
			COLOR_YELLOW ")\n" COLOR_NC, scene->samples, scene->max_bounces);
		pthread_create(&scene->thread, &thread_attr, render, scene);
		if (scene->window == NULL)
		{
			pthread_join(scene->thread, NULL);
			clean_exit(scene, 0);
		}
	}
	if (scene->mlx != NULL && scene->window != NULL)
		mlx_put_image_to_window(scene->mlx, scene->window, scene->img.img,
			0, 0);
	return (0);
}
