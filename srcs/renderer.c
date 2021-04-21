/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   renderer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/14 11:55:19 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/21 10:46:41 by ejuliao-         ###   ########.fr       */
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
	}
	return (false);
}

t_ray	gen_ray(t_scene scene, float u, float v, t_vec3 origin, t_vec3 dir)
{
	t_ray	ray;

	ray.origin.x = origin.x;
	ray.origin.y = origin.y;
	ray.origin.z = origin.z;
	ray.direction.x = -((float)scene.x_res / (float)scene.y_res) +
			dir.x + (u * ((float)scene.x_res
			/ (float)scene.y_res) * 2.0f);
	ray.direction.y = -1.0f + dir.y + (v * 2.0f);
	ray.direction.z = 1.0f + dir.z;
	ray.direction.x = - ray.direction.x;
	ray.direction.y = - ray.direction.y;
	return (ray);
}

void	gen_pixel_clr(t_ray ray, t_color *hit_color, float t)
{
	t_vec3	n;

	n.x = ray.origin.x + t * ray.direction.x;
	n.y = ray.origin.y + t * ray.direction.y;
	n.z = ray.origin.z + t * ray.direction.z;
	n = normalize(n);
	t = (0.5f * n.z + 0.3f) * 255.0f;
	set_color(hit_color, hit_color->r - t, hit_color->g - t,
			hit_color->b - t);
}

t_vec3	random_in_unit_sphere()
{
	t_vec3	p;

	while (true)
	{
		p.x = 2.0f * drand48() - 1.0f;
		p.y = 2.0f * drand48() - 1.0f;
		p.z = 2.0f * drand48() - 1.0f;
		if (p.x * p.x + p.y * p.y + p.z * p.z >= 1.0f)
			break ;
	}
	return (p);
}

bool	check_ray_hits(t_holder *holder, t_ray ray, t_color *hit_color, t_hit_record *hit_record)
{
	float			closest;
	bool			hit;

	hit = false;
	closest = holder->scene.t_max;
	while (true)
	{
		if (hit_sphere(holder->scene, &ray, hit_record, closest))
		{
			hit = true;
			*hit_color = holder->scene.sphere->color;
			gen_pixel_clr(ray, hit_color, hit_record->t);
			closest = hit_record->t;
		}
		if (holder->scene.sphere->next == NULL)
			break ;
		else
			holder->scene.sphere = holder->scene.sphere->next;
	}
	while (holder->scene.sphere->prev != NULL)
		holder->scene.sphere = holder->scene.sphere->prev;
	return (hit);
}

void	render(t_img *img_data, int width, int height, t_holder *holder)
{
	int		x;
	int		y;
	int		aa_s;
	t_color	hit_color;
	t_color	tmp_color;

	y = 0;
	while (y < height)
	{
		x = 0;
		while (x < width)
		{
			aa_s = 0;
			while (aa_s < holder->scene.aa_samples)
			{
				t_hit_record	hit_record;
				set_color(&tmp_color, 255, 255, 255); // Background color
				if (check_ray_hits(holder, gen_ray(holder->scene, (float)(x
						+ drand48()) / (float)width, (float)(y + drand48())
						/ (float)height, holder->scene.camera.transform.position,
						holder->scene.camera.transform.orientation), &tmp_color, &hit_record))
				{
					t_vec3 random = random_in_unit_sphere();
					t_vec3 target;
					target.x = hit_record.p.x + hit_record.normal.x + random.x;
					target.y = hit_record.p.y + hit_record.normal.y + random.y;
					target.z = hit_record.p.z + hit_record.normal.z + random.z;
					t_vec3 tp;
					tp.x = target.x - hit_record.p.x;
					tp.y = target.y - hit_record.p.y;
					tp.z = target.z - hit_record.p.z;
					check_ray_hits(holder, gen_ray(holder->scene, (float)(x
						+ drand48()) / (float)width, (float)(y + drand48())
						/ (float)height, hit_record.p, tp),
						&tmp_color, &hit_record);
				}
				set_color(&hit_color, hit_color.r + tmp_color.r, hit_color.g + tmp_color.g, hit_color.b + tmp_color.b);
				aa_s++;
			}
			set_color(&hit_color, hit_color.r / aa_s, hit_color.g / aa_s, hit_color.b / aa_s);
			put_pixel(img_data, x, y, rgba_to_hex(hit_color));
			//mlx_pixel_put(holder->mlx, holder->window, x, y, rgba_to_hex(hit_color));
			x++;
		}
		y++;
	}
}
