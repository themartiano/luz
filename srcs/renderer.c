/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 11:56:26 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	check_ray_hits(t_holder *holder, t_ray ray, t_color *hit_color,
t_hit_record *hit_record)
{
	float	closest;
	bool	hit;

	hit = false;
	closest = holder->scene.t_max;
	while (true)
	{
		if (hit_sphere(holder->scene, &ray, hit_record, closest))
		{
			hit = true;
			*hit_color = holder->scene.sphere->color;
			gen_pixel_clr(holder, ray, hit_color, hit_record->t);
			closest = hit_record->t;
		}
		if (holder->scene.sphere->next == NULL)
			break ;
		else
			holder->scene.sphere = holder->scene.sphere->next;
	}
	while (holder->scene.sphere->prev != NULL)
		holder->scene.sphere = holder->scene.sphere->prev;
	return (hit);
}

void	light_bouncer(t_holder *holder, t_vec3 uv, t_color *tmp_color,
t_hit_record *hit_record)
{
	t_vec3	tmp_vec3;
	t_vec3	target;
	int		bounces;

	bounces = 0;
	while (bounces < holder->scene.max_bounces)
	{
		target = random_in_unit();
		tmp_vec3.x = hit_record->p.x + hit_record->normal.x + target.x;
		tmp_vec3.y = hit_record->p.y + hit_record->normal.y + target.y;
		tmp_vec3.z = hit_record->p.z + hit_record->normal.z + target.z;
		target.x = tmp_vec3.x - hit_record->p.x;
		target.y = tmp_vec3.y - hit_record->p.y;
		target.z = tmp_vec3.z - hit_record->p.z;
		if (!check_ray_hits(holder, gen_ray(holder->scene, uv,
					hit_record->p, target), tmp_color, hit_record))
		{
			break ;
		}
		bounces++;
	}
}

void	get_hit_color(t_holder *holder, t_hit_record *hit_record,
t_color *tmp_color, t_vec3 crnt_pxl)
{
	t_vec3	uv;

	uv.x = (float)(crnt_pxl.x + drand48()) / (float)holder->scene.x_res;
	uv.y = (float)(crnt_pxl.y + drand48()) / (float)holder->scene.y_res;
	if (check_ray_hits(holder, gen_ray(holder->scene, uv,
				holder->scene.camera.transform.position,
				holder->scene.camera.transform.orientation), tmp_color,
			hit_record))
	{
		light_bouncer(holder, uv, tmp_color, hit_record);
	}
}

void	render_loop(t_holder *holder, t_color *hit_color, int x, int y)
{
	t_hit_record	hit_record;
	t_vec3			current_pixel;
	t_color			tmp_color;
	int				s;

	current_pixel.x = x;
	current_pixel.y = y;
	s = 0;
	while (s < holder->scene.samples)
	{
		set_color(&tmp_color, 255, 255, 255);
		get_hit_color(holder, &hit_record, &tmp_color, current_pixel);
		set_color(hit_color, hit_color->r + tmp_color.r,
			hit_color->g + tmp_color.g, hit_color->b + tmp_color.b);
		s++;
	}
	set_color(hit_color, hit_color->r / s, hit_color->g / s,
		hit_color->b / s);
}

void	start_render(t_holder *holder)
{
	int		x;
	int		y;
	t_color	hit_color;

	y = 0;
	while (y < holder->scene.y_res)
	{
		x = 0;
		while (x < holder->scene.x_res)
		{
			render_loop(holder, &hit_color, x, y);
			put_pixel(&holder->img, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}
