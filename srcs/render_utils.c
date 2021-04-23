/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/23 10:05:52 by ejuliao-         ###   ########.fr       */
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

void	gen_pixel_clr(t_holder *holder, t_ray ray, t_color *hit_color, float t)
{
	t_vec3	n;

	n = set(ray.origin.x + t * ray.direction.x, ray.origin.y + t
			* ray.direction.y, ray.origin.z + t * ray.direction.z);
	n = normalize(n);
	t = (0.5f * n.z + 0.1f) * 255.0f;
	set_color(hit_color,
		((float)hit_color->r - t) + ((float)holder->scene.amb_light.color.r
			* holder->scene.amb_light.brightness),
		((float)hit_color->g - t) + ((float)holder->scene.amb_light.color.g
			* holder->scene.amb_light.brightness),
		((float)hit_color->b - t) + ((float)holder->scene.amb_light.color.b
			* holder->scene.amb_light.brightness));
}

t_ray	gen_ray(t_scene scene, t_vec3 uv, t_vec3 origin, t_vec3 dir)
{
	t_ray	ray;
	float	theta;
	float	half_height;
	float	half_width;

	theta = scene.camera.fov * M_PI / 180;
	half_height = tan(theta / 2);
	half_width = ((float)scene.x_res / (float)scene.y_res) * half_height;
	ray.origin.x = origin.x;
	ray.origin.y = origin.y;
	ray.origin.z = origin.z;
	ray.direction.x = -half_width + dir.x + (uv.x * half_width * 2.0f);
	ray.direction.y = -half_height + dir.y + (uv.y * half_height * 2.0f);
	ray.direction.z = 1.0f + dir.z;
	ray.direction.x = ray.direction.x * -1.0f;
	ray.direction.y = ray.direction.y * -1.0f;
	return (ray);
}

bool	check_ray_hits(t_holder *holder, t_ray ray, t_color *hit_color,
t_hit_record *hit_rec)
{
	float	closest;
	bool	hit;

	hit = false;
	closest = holder->scene.t_max;
	while (true)
	{
		if (holder->scene.objects->type == 0
			&& hit_sphere(holder->scene, &ray, hit_rec, closest))
		{
			hit = true;
			*hit_color = get_sphere(holder->scene)->color;
			gen_pixel_clr(holder, ray, hit_color, hit_rec->t);
			closest = hit_rec->t;
		}
		if (holder->scene.objects->next == NULL)
			break ;
		else
			holder->scene.objects = holder->scene.objects->next;
	}
	while (holder->scene.objects->prev != NULL)
		holder->scene.objects = holder->scene.objects->prev;
	return (hit);
}
