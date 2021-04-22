/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 18:56:01 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	check_ray_hits(t_holder *holder, t_ray ray, t_color *hit_color,
t_hit_record *hit_rec)
{
	float	closest;
	bool	hit;

	hit = false;
	closest = holder->scene.t_max;
	while (true)
	{
		if (hit_sphere(holder->scene, &ray, hit_rec, closest))
		{
			hit = true;
			*hit_color = holder->scene.sphere->color;
			gen_pixel_clr(holder, ray, hit_color, hit_rec->t);
			closest = hit_rec->t;
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

void	light_bouncer(t_holder *holder, t_vec3 uv, t_color *hit_color,
t_hit_record *hit_rec)
{
	t_vec3	tmp_vec3;
	t_vec3	target;
	int		bounces;
	int		attempts;

	bounces = 0;
	attempts = 0;
	while (bounces < holder->scene.max_bounces)
	{
		tmp_vec3 = sum(sum(hit_rec->p, hit_rec->normal), random_in_unit());
		target = sub(tmp_vec3, hit_rec->p);
		if (!check_ray_hits(holder, gen_ray(holder->scene, uv,
					hit_rec->p, target), hit_color, hit_rec))
		{
			if (bounces >= holder->scene.min_bounces
				|| attempts >= holder->scene.min_bounces * 2)
				break ;
			else
			{
				attempts++;
				continue ;
			}
		}
		bounces++;
	}
}

bool	get_hit_color(t_holder *holder, t_hit_record *hit_rec,
t_color *hit_color, t_vec3 crnt_pxl)
{
	t_vec3	uv;

	uv.x = (float)(crnt_pxl.x + drand48()) / (float)holder->scene.x_res;
	uv.y = (float)(crnt_pxl.y + drand48()) / (float)holder->scene.y_res;
	if (check_ray_hits(holder, gen_ray(holder->scene, uv,
				holder->scene.camera.transform.position,
				holder->scene.camera.transform.orientation), hit_color,
			hit_rec))
	{
		light_bouncer(holder, uv, hit_color, hit_rec);
		return (true);
	}
	else
		return (false);
}

void	render_loop(t_holder *holder, t_color *hit_color, t_vec3 current_pixel, int s)
{
	t_hit_record	hit_rec;
	t_color			pxl_color;

	set_color(hit_color, 255, 255, 255);
	if (get_hit_color(holder, &hit_rec, hit_color, current_pixel)
		&& s > 0)
	{
		pxl_color = get_pixel(&holder->img, current_pixel.x, current_pixel.y);
		set_color(hit_color, (hit_color->r + pxl_color.r) / 2,
			(hit_color->g + pxl_color.g) / 2, (hit_color->b + pxl_color.b) / 2);
	}
}

void	render(t_holder *holder, int s)
{
	int		x;
	int		y;
	t_color	hit_color;
	t_vec3	current_pixel;

	y = 0;
	while (y < holder->scene.y_res)
	{
		x = 0;
		while (x < holder->scene.x_res)
		{
			current_pixel.x = x;
			current_pixel.y = y;
			render_loop(holder, &hit_color, current_pixel, s);
			put_pixel(&holder->img, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}

int	start_render(t_holder *holder)
{
	static unsigned int	frame = 0;
	static int			s = 0;

	if (frame == 0)
	{
		mlx_string_put(holder->mlx, holder->window, 20, 20, 0x00FFFFFF,
			"Rendering...");
	}
	if (frame >= 2 && s < holder->scene.samples)
	{
		render(holder, s);
		mlx_put_image_to_window(holder->mlx, holder->window, holder->img.img,
			0, 0);
		s++;
		if (s == holder->scene.samples)
			printf("Rendering done.\n");
	}
	frame++;
	return (0);
}
