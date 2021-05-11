/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/10 19:26:02 by ejuliao-         ###   ########.fr       */
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
				scene->camera.transform.position,
				scene->camera.transform.orientation),
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

static float	manage_hit(t_scene *scene, t_hit_record *hit_rec)
{
	hit_rec->hit = true;
	hit_rec->color = set_color(
			((float)hit_rec->color.r) + ((float)scene->amb_light.color.r
				* scene->amb_light.brightness),
			((float)hit_rec->color.g) + ((float)scene->amb_light.color.g
				* scene->amb_light.brightness),
			((float)hit_rec->color.b) + ((float)scene->amb_light.color.b
				* scene->amb_light.brightness));
	return (hit_rec->t);
}

bool	check_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
{
	float	closest;

	hit_rec->hit = false;
	closest = scene->t_max;
	while (true)
	{
		if (scene->objects == NULL)
			break ;
		if (((scene->objects->type == 0
					&& hit_sphere(scene, &ray, hit_rec, closest))
				|| (scene->objects->type == 1
					&& hit_plane(scene, &ray, hit_rec, closest))
				|| (scene->objects->type == 2
					&& hit_square(scene, &ray, hit_rec, closest))
				|| (scene->objects->type == 3
					&& hit_cylinder(scene, &ray, hit_rec, closest))
				|| (scene->objects->type == 4
					&& hit_triangle(scene, &ray, hit_rec, closest))))
			closest = manage_hit(scene, hit_rec);
		if (scene->objects->next == NULL)
			break ;
		else
			scene->objects = scene->objects->next;
	}
	if (scene->objects != NULL)
		while (scene->objects->prev != NULL)
			scene->objects = scene->objects->prev;
	return (hit_rec->hit);
}
