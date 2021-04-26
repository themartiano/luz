/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 17:25:44 by ejuliao-         ###   ########.fr       */
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

t_generic_object	*get_generic_object(t_scene scene)
{
	t_generic_object	*generic_obj;

	generic_obj = scene.objects->object;
	return (generic_obj);
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
		brightness = (get_generic_object(scene)->color.r + get_generic_object(scene)->color.g
				+ get_generic_object(scene)->color.b) / 765.0f;
		random = drand48();
		if (brightness < random - 0.001f || brightness > random + 0.001f)
			light_bouncer(scene, uv, hit_color, hit_rec);
		return (true);
	}
	else
		return (false);
}

static void	gen_pixel_clr(t_scene scene, t_ray ray, t_color *hit_color, float t)
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
		else if (scene.objects->type == 1
			&& hit_plane(scene, &ray, hit_rec, closest))
		{
			hit = true;
			*hit_color = get_plane(scene)->color;
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
