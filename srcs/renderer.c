/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 09:36:09 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	light_bouncer(t_scene scene, t_vec3 uv, t_color *hit_color,
t_hit_record *hit_rec)
{
	t_vec3	tmp_vec3;
	t_vec3	target;
	int		bounces;

	bounces = 0;
	while (bounces < scene.max_bounces)
	{
		tmp_vec3 = sum(sum(hit_rec->p, hit_rec->normal), random_in_unit());
		target = sub(tmp_vec3, hit_rec->p);
		target.x = 0;
		target.y = 0;
		target.z -= 1.0f;
		if (!check_ray_hits(scene, gen_ray(scene, uv,
					hit_rec->p, target), hit_color, hit_rec))
		{
			break ;
		}
		bounces++;
	}
}

t_ray	gen_ray(t_scene scene, t_vec3 uv, t_vec3 origin, t_vec3 dir)
{
	t_ray	ray;
	t_vec3	view_up;
	t_vec3	w;
	t_vec3	u;
	t_vec3	v;

	view_up = set(0, 1, 0);
	w = unit_vector(sub(origin, dir));
	u = unit_vector(cross(view_up, w));
	v = cross(w, u);
	ray.origin.x = origin.x;
	ray.origin.y = origin.y;
	ray.origin.z = origin.z;
	ray.direction.x = -scene.camera.half_width + dir.x + (uv.x * u.x
			* scene.camera.half_width * 2.0f);
	ray.direction.y = -scene.camera.half_height + dir.y + (uv.y * v.y
			* scene.camera.half_height * 2.0f);
	ray.direction.z = dir.z;
	ray.direction = normalize(ray.direction);
	ray.direction.y = -ray.direction.y;
	(void)uv;
	return (ray);
}


static void	render_loop(t_scene scene, t_color *hit_color, t_vec3 current_pixel)
{
	t_hit_record	hit_rec;
	t_color			tmp_color;
	int				s;

	s = 0;
	while (s < scene.samples)
	{
		set_color(&tmp_color, 255, 255, 255);
		get_hit_color(scene, &hit_rec, &tmp_color, current_pixel);
		set_color(hit_color, hit_color->r + tmp_color.r,
			hit_color->g + tmp_color.g, hit_color->b + tmp_color.b);
		s++;
	}
	set_color(hit_color, hit_color->r / s, hit_color->g / s,
		hit_color->b / s);
}

void	*render(void *vargp)
{
	t_holder	*holder;
	t_color		hit_color;
	t_vec3		current_pixel;
	int			x;
	int			y;

	holder = (t_holder *)vargp;
	y = 0;
	while (y < holder->scene.y_res)
	{
		x = 0;
		while (x < holder->scene.x_res)
		{
			current_pixel.x = x;
			current_pixel.y = y;
			render_loop(holder->scene, &hit_color, current_pixel);
			put_pixel(&holder->img, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "Render done at: %s", asctime (timeinfo) );
	return (NULL);
}

int	manage_frames(t_holder *holder)
{
	static pthread_t	thread_id = 0;

	if (thread_id == 0)
	{
		time_t rawtime;
		struct tm * timeinfo;

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ( "Render started at: %s", asctime (timeinfo) );
		pthread_create(&thread_id, NULL, render, holder);
	}
	mlx_put_image_to_window(holder->mlx, holder->window, holder->img.img, 0, 0);

	return (0);
}
