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

float	hit_sphere(t_sphere sphere, t_ray ray)
{
	t_vec3	oc;
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
	d = b * b - 4.0f * a * c;
	if (d < 0.0f)
		return (-1.0f);
	else
		return ((-b - sqrt(d)) / (2.0f * a));
}

t_ray	gen_ray(float u, float v, int width, int height)
{
	t_ray	ray;

	ray.origin.x = 0;
	ray.origin.y = 0;
	ray.origin.z = 0;
	ray.direction.x = -((float)width / (float)height) + (u
			* ((float)width / (float)height) * 2.0f);
	ray.direction.y = -1.0f + (v * 2.0f);
	ray.direction.z = -1;
	return (ray);
}

void	gen_pixel_clr(t_ray ray, t_color *hit_color, float t)
{
	t_vec3	n;

	n.x = ray.origin.x + t * ray.direction.x;
	n.y = ray.origin.y + t * ray.direction.y;
	n.z = ray.origin.z + t * ray.direction.z;
	n = normalize(n);
	t = (0.5f * n.z + 1.0f) * 255.0f;
	set_color(hit_color, hit_color->r - t, hit_color->g - t,
		   hit_color->b - t);
}

void	check_ray_hits(t_holder *holder, t_ray ray, t_color *hit_color)
{
	float	t;

	t = hit_sphere(holder->scene.sphere, ray);
	if (t < -1.0f)
	{
		*hit_color = holder->scene.sphere.color;
		gen_pixel_clr(ray, hit_color, t);
	}
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
			set_color(&hit_color, 255, 255, 255);
			ray = gen_ray((float)x / (float)width, (float)y / (float)height,
					width, height);
			check_ray_hits(holder, ray, &hit_color);
			put_pixel(img_data, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}
