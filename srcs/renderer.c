/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 19:08:34 by ejuliao-         ###   ########.fr       */
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
		if (!check_ray_hits(scene, gen_ray(scene, uv,
					hit_rec->p, target), hit_color, hit_rec))
		{
			break ;
		}
		bounces++;
	}
}

bool	get_hit_color(t_scene scene, t_hit_record *hit_rec,
t_color *hit_color, t_vec3 crnt_pxl)
{
	t_vec3	uv;
	float	brightness;
	float	random;

	uv.x = (float)(crnt_pxl.x + drand48()) / (float)scene.x_res;
	uv.y = (float)(crnt_pxl.y + drand48()) / (float)scene.y_res;
	if (check_ray_hits(scene, gen_ray(scene, uv,
				scene.camera.transform.position,
				scene.camera.transform.orientation), hit_color,
			hit_rec))
	{
		brightness = (get_sphere(scene)->color.r + get_sphere(scene)->color.g
				+ get_sphere(scene)->color.b) / 765.0f;
		random = drand48();
		if (brightness < random - 0.001f || brightness > random + 0.001f)
			light_bouncer(scene, uv, hit_color, hit_rec);
		return (true);
	}
	else
		return (false);
}

void	render_loop(t_holder *holder, t_color *hit_color, t_vec3 current_pixel)
{
	t_hit_record	hit_rec;
	t_color			tmp_color;
	int				s;

	s = 0;
	while (s < holder->scene.samples)
	{
		set_color(&tmp_color, 255, 255, 255);
		get_hit_color(holder->scene, &hit_rec, &tmp_color, current_pixel);
		set_color(hit_color, hit_color->r + tmp_color.r,
			hit_color->g + tmp_color.g, hit_color->b + tmp_color.b);
		s++;
	}
	set_color(hit_color, hit_color->r / s, hit_color->g / s,
		hit_color->b / s);
}

void	render(t_holder *holder)
{
	t_color	hit_color;
	t_vec3	current_pixel;
	int		x;
	int		y;

	y = 0;
	while (y < holder->scene.y_res)
	{
		x = 0;
		while (x < holder->scene.x_res)
		{
			current_pixel.x = x;
			current_pixel.y = y;
			render_loop(holder, &hit_color, current_pixel);
			put_pixel(&holder->img, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}

int	start_render(t_holder *holder)
{
	static unsigned int	frame = 0;
	static bool			rendered = false;

	if (frame == 0)
	{
		mlx_string_put(holder->mlx, holder->window, 20, 20, 0x00FFFFFF,
			"Rendering...");
	}
	if (rendered == false && frame >= 2)
	{
		render(holder);
		mlx_put_image_to_window(holder->mlx, holder->window, holder->img.img,
			0, 0);
		rendered = true;
	}
	frame++;
	return (0);
}
