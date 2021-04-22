/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 11:08:20 by ejuliao-         ###   ########.fr       */
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

	ray.origin.x = origin.x;
	ray.origin.y = origin.y;
	ray.origin.z = origin.z;
	ray.direction.x = -((float)scene.x_res / (float)scene.y_res) + dir.x + (uv.x
			* ((float)scene.x_res / (float)scene.y_res) * 2.0f);
	ray.direction.y = -1.0f + dir.y + (uv.y * 2.0f);
	ray.direction.z = 1.0f + dir.z;
	ray.direction.x = ray.direction.x * -1.0f;
	ray.direction.y = ray.direction.y * -1.0f;
	return (ray);
}
