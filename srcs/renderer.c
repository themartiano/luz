/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/14 11:55:20 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	hit_sphere(t_sphere sphere, t_ray ray)
{
	t_xyz	oc;
	float	a;
	float	b;
	float	c;
	float	d;

	oc.x = ray.origin.x - sphere.transform.position.x;
	oc.y = ray.origin.y - sphere.transform.position.y;
	oc.z = ray.origin.z - sphere.transform.position.z;
	a = dot(ray.direction, ray.direction);
	b = 2.0f * dot(oc, ray.direction);
	c = dot(oc, oc) - (sphere.diameter / 2.0f) * (sphere.diameter / 2.0f);
	d = b * b - 4 * a * c;
	return (d < 0);
}

t_ray	gen_ray(float u, float v)
{
	t_ray	ray;

	ray.origin.x = 0;
	ray.origin.y = 0;
	ray.origin.z = 0;
	ray.direction.x = -2.0f + (u * 4.0f);
	ray.direction.y = -1.0f + v * 2.0f;
	ray.direction.z = -1;
	return (ray);
}

void	render(t_img *img_data, int width, int height, t_holder *holder)
{
	int		x;
	int		y;
	t_color	hit_color;
	t_ray	ray;

	y = 0;
	while (y < height)
	{
		x = 0;
		while (x < width)
		{
			hit_color.r = 255;
			hit_color.g = 255;
			hit_color.b = 255;
			ray = gen_ray((float)x / (float)width, (float)y / (float)height);
			if (hit_sphere(holder->scene.sphere, ray))
				hit_color = holder->scene.sphere.color;
			put_pixel(img_data, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}
