/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/21 12:01:38 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_vec3	random_in_unit_sphere(void)
{
	t_vec3	p;
	float	tmp;

	while (true)
	{
		tmp = 2.0f * drand48() - 1.0f;
		p = set(tmp, tmp, tmp);
		if (p.x * p.x + p.y * p.y + p.z * p.z >= 1.0f)
			break ;
	}
	return (p);
}

void	gen_pixel_clr(t_ray ray, t_color *hit_color, float t)
{
	t_vec3	n;

	n = set(ray.origin.x + t * ray.direction.x, ray.origin.y + t
			* ray.direction.y, ray.origin.z + t * ray.direction.z);
	n = normalize(n);
	t = (0.5f * n.z + 0.3f) * 255.0f;
	set_color(hit_color, hit_color->r - t, hit_color->g - t, hit_color->b - t);
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
