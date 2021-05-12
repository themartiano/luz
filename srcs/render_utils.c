/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/12 16:42:28 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

bool	get_hit_color(t_scene *scene, t_hit_record *hit_rec, int x, int y)
{
	t_vec2	pxl;
	float	brightness;
	float	random;

	pxl.x = (float)(x + drand48()) / (float)scene->x_res;
	pxl.y = (float)(y + drand48()) / (float)scene->y_res;
	if (check_ray_hits(scene, gen_ray(scene, pxl,
				get_camera(scene)->transform.position,
				get_camera(scene)->transform.orientation),
			hit_rec))
	{
		brightness = (hit_rec->color.r
				+ hit_rec->color.g
				+ hit_rec->color.b) / 765.0f;
		random = drand48();
		if (brightness < random - 0.001f || brightness > random + 0.001f)
			light_bouncer(scene, pxl, hit_rec);
		return (true);
	}
	else
		return (false);
}

bool	check_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
{
	float	closest;
	bool	hit;

	hit_rec->hit = false;
	closest = scene->t_max;
	while (true)
	{
		if (scene->objects == NULL)
			break ;
		hit_rec->hit_color = set_color(0, 0, 0);
		hit = false;
		if (scene->objects->type == 0)
			hit = hit_sphere(scene, &ray, hit_rec, closest);
		else if (scene->objects->type == 1)
			hit = hit_plane(scene, &ray, hit_rec, closest);
		else if (scene->objects->type == 2)
			hit = hit_square(scene, &ray, hit_rec, closest);
		else if (scene->objects->type == 3)
			hit = hit_cylinder(scene, &ray, hit_rec, closest);
		else if (scene->objects->type == 4)
			hit = hit_triangle(scene, &ray, hit_rec, closest);
		if (hit == true)
		{
			hit_rec->hit = true;
			closest = hit_rec->t;
		}
		if (scene->objects->next == NULL)
			break ;
		else
			scene->objects = scene->objects->next;
	}
	if (hit_rec->hit == true)
	{
		set_hit_color(scene, hit_rec);
	}
	while (scene->objects != NULL && scene->objects->prev != NULL)
		scene->objects = scene->objects->prev;
	return (hit_rec->hit);
}
