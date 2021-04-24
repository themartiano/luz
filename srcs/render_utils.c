/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 19:27:23 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_vec3	random_in_unit(void)
{
	float	tmp;

	while (true)
	{
		tmp = 2.0f * drand48() - 1.0f;
		if (tmp * tmp + tmp * tmp + tmp * tmp >= 1.0f)
			break ;
	}
	return (set(tmp, tmp, tmp));
}

void	gen_pixel_clr(t_scene scene, t_ray ray, t_color *hit_color, float t)
{
	t_vec3	n;
	float	shadow_level;

	shadow_level = 0.3f;
	n = set(ray.origin.x + t * ray.direction.x, ray.origin.y + t
			* ray.direction.y, ray.origin.z + t * ray.direction.z);
	n = normalize(n);
	t = (0.5f * n.z + shadow_level) * 255.0f;
	set_color(hit_color,
		((float)hit_color->r - t) + ((float)scene.amb_light.color.r
			* scene.amb_light.brightness),
		((float)hit_color->g - t) + ((float)scene.amb_light.color.g
			* scene.amb_light.brightness),
		((float)hit_color->b - t) + ((float)scene.amb_light.color.b
			* scene.amb_light.brightness));
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

bool	check_ray_hits(t_scene scene, t_ray ray, t_color *hit_color,
t_hit_record *hit_rec)
{
	float	closest;
	bool	hit;

	hit = false;
	closest = scene.t_max;
	while (true)
	{
		if (scene.objects->type == 0
			&& hit_sphere(scene, &ray, hit_rec, closest))
		{
			hit = true;
			*hit_color = get_sphere(scene)->color;
			gen_pixel_clr(scene, ray, hit_color, hit_rec->t);
			closest = hit_rec->t;
		}
		if (scene.objects->next == NULL)
			break ;
		else
			scene.objects = scene.objects->next;
	}
	while (scene.objects->prev != NULL)
		scene.objects = scene.objects->prev;
	return (hit);
}
