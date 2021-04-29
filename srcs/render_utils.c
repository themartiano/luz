/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 12:38:24 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

bool	get_hit_color(t_scene *scene, t_hit_record *hit_rec, int x, int y)
{
	t_vec3	uv;
	float	brightness;
	float	random;

	uv.x = (float)(x + drand48()) / (float)scene->x_res;
	uv.y = (float)(y + drand48()) / (float)scene->y_res;
	if (check_ray_hits(scene, gen_ray(scene, uv,
				scene->camera.transform.position,
				scene->camera.transform.orientation),
			hit_rec))
	{
		brightness = (hit_rec->color.r
				+ hit_rec->color.g
				+ hit_rec->color.b) / 765.0f;
		random = drand48();
		if (brightness < random - 0.001f || brightness > random + 0.001f)
			light_bouncer(scene, uv, hit_rec);
		return (true);
	}
	else
		return (false);
}

static void	gen_pixel_clr(t_scene *scene, t_ray ray, t_color *hit_color,
float t)
{
	t_vec3	n;
	float	shadow_level;

	shadow_level = 0.3f;
	n = set(ray.origin.x + t * ray.direction.x, ray.origin.y + t
			* ray.direction.y, ray.origin.z + t * ray.direction.z);
	n = normalize(n);
	t = (0.5f * n.z + shadow_level) * 255.0f;
	*hit_color = set_color(
			((float)hit_color->r - t) + ((float)scene->amb_light.color.r
				* scene->amb_light.brightness),
			((float)hit_color->g - t) + ((float)scene->amb_light.color.g
				* scene->amb_light.brightness),
			((float)hit_color->b - t) + ((float)scene->amb_light.color.b
				* scene->amb_light.brightness));
}

static float	manage_hit(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
{
	hit_rec->hit = true;
	gen_pixel_clr(scene, ray, &hit_rec->color, hit_rec->t);
	return (hit_rec->t);
}

bool	check_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
{
	float	closest;

	hit_rec->hit = false;
	closest = scene->t_max;
	while (true)
	{
		if (scene->objects != NULL && ((scene->objects->type == 0
					&& hit_sphere(scene, &ray, hit_rec, closest))
				|| (scene->objects->type == 1
					&& hit_plane(scene, &ray, hit_rec, closest))
				|| (scene->objects->type == 3
					&& hit_cylinder(scene, &ray, hit_rec, closest))))
			closest = manage_hit(scene, ray, hit_rec);
		if (scene->objects->next == NULL)
			break ;
		else
			scene->objects = scene->objects->next;
	}
	while (scene->objects->prev != NULL)
		scene->objects = scene->objects->prev;
	return (hit_rec->hit);
}
