/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/20 11:21:18 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	hit_sphere(t_scene scene, t_ray *ray, t_hit_record *hit_record, float t_max)
{
	t_vec3	oc;
	float	a;
	float	b;
	float	c;
	float	d;

	oc.x = ray->origin.x - scene.sphere->transform.position.x;
	oc.y = ray->origin.y - scene.sphere->transform.position.y;
	oc.z = ray->origin.z - scene.sphere->transform.position.z;
	a = dot(ray->direction, ray->direction);
	b = dot(oc, ray->direction);
	c = dot(oc, oc) - scene.sphere->radius * scene.sphere->radius;
	d = b * b - a * c;
	if (d > 0)
	{
		float temp = (-b - sqrt(b * b - a * c)) / a;
		if (temp < t_max && temp > scene.t_min)
		{
			hit_record->t = temp;
			hit_record->p.x = ray->origin.x + hit_record->t * ray->direction.x;
			hit_record->p.y = ray->origin.y + hit_record->t * ray->direction.y;
			hit_record->p.z = ray->origin.z + hit_record->t * ray->direction.z;
			hit_record->normal.x = (hit_record->p.x - scene.sphere->transform.position.x) / scene.sphere->radius;
			hit_record->normal.y = (hit_record->p.y - scene.sphere->transform.position.y) / scene.sphere->radius;
			hit_record->normal.z = (hit_record->p.z - scene.sphere->transform.position.z) / scene.sphere->radius;
			return (true);
		}
		temp = (-b + sqrt(b * b - a * c)) / a;
		if (temp < t_max && temp > scene.t_min)
		{
			hit_record->t = temp;
			hit_record->p.x = ray->origin.x + hit_record->t * ray->direction.x;
			hit_record->p.y = ray->origin.y + hit_record->t * ray->direction.y;
			hit_record->p.z = ray->origin.z + hit_record->t * ray->direction.z;
			hit_record->normal.x = (hit_record->p.x - scene.sphere->transform.position.x) / scene.sphere->radius;
			hit_record->normal.y = (hit_record->p.y - scene.sphere->transform.position.y) / scene.sphere->radius;
			hit_record->normal.z = (hit_record->p.z - scene.sphere->transform.position.z) / scene.sphere->radius;
			return (true);
		}
	}
	return (false);
}

t_ray	gen_ray(t_scene scene, float u, float v)
{
	t_ray	ray;

	ray.origin.x = scene.camera.transform.position.x;
	ray.origin.y = scene.camera.transform.position.y;
	ray.origin.z = scene.camera.transform.position.z;
	ray.direction.x = -((float)scene.x_res / (float)scene.y_res) +
			scene.camera.transform.orientation.x + (u * ((float)scene.x_res
			/ (float)scene.y_res) * 2.0f);
	ray.direction.y = -1.0f + scene.camera.transform.orientation.y + (v * 2.0f);
	ray.direction.z = -1.0f + scene.camera.transform.orientation.z;
	ray.direction.x = - ray.direction.x;
	ray.direction.y = - ray.direction.y;
	ray.direction.z = - ray.direction.z;
	return (ray);
}

void	gen_pixel_clr(t_ray ray, t_color *hit_color, float t)
{
	t_vec3	n;

	n.x = ray.origin.x + t * ray.direction.x;
	n.y = ray.origin.y + t * ray.direction.y;
	n.z = ray.origin.z + t * ray.direction.z;
	n = normalize(n);
	t = (0.5f * n.z + OS_BRIGHTNESS_FACTOR) * 255.0f;
	set_color(hit_color, hit_color->r - t, hit_color->g - t,
			-hit_color->b - t);
}

void	check_ray_hits(t_holder *holder, t_ray ray, t_color *hit_color)
{
	t_hit_record	hit_record;
	float			closest;

	closest = holder->scene.t_max;
	while (true)
	{
		if (hit_sphere(holder->scene, &ray, &hit_record, closest))
		{
			*hit_color = holder->scene.sphere->color;
			gen_pixel_clr(ray, hit_color, hit_record.t);
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
			//mlx_pixel_put(holder->mlx, holder->window, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}
