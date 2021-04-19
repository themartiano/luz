/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/19 15:59:43 by ejuliao-         ###   ########.fr       */
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
	c = dot(oc, oc) - sphere.radius * sphere.radius;
	d = b * b - 4.0f * a * c;
	if (d < 0.0f)
		return (-1.0f);
	else
		return ((-b - sqrt(d)) / (2.0f * a));
}

t_ray	gen_ray(t_scene scene, float u, float v)
{
	t_ray	ray;

	ray.origin.x = scene.camera.transform.position.x;
	ray.origin.y = scene.camera.transform.position.y;
	ray.origin.z = scene.camera.transform.position.z;
	ray.origin.z = -(ray.origin.z);
	ray.direction.x = -((float)scene.x_res / (float)scene.y_res) +
			scene.camera.transform.orientation.x + (u * ((float)scene.x_res
			/ (float)scene.y_res) * 2.0f);
	ray.direction.y = -1.0f + scene.camera.transform.orientation.y + (v * 2.0f);
	ray.direction.z = -1.0f + scene.camera.transform.orientation.z;
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
	
	while (true)
	{
		t = hit_sphere(*holder->scene.sphere, ray);
		if (t < -1.0f)
		{
			*hit_color = holder->scene.sphere->color;
			gen_pixel_clr(ray, hit_color, t);
		}
		if (holder->scene.sphere->next == NULL)
			break ;
		else
			holder->scene.sphere = holder->scene.sphere->next;
	}
	while (holder->scene.sphere->prev != NULL)
		holder->scene.sphere = holder->scene.sphere->prev;
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
			ray = gen_ray(holder->scene, (float)x / (float)width,
					(float)y / (float)height);
			check_ray_hits(holder, ray, &hit_color);
			put_pixel(img_data, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}
