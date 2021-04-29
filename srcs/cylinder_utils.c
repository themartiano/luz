/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cylinder_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 09:46:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 19:26:35 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_cylinder	*get_cylinder(t_scene *scene)
{
	t_cylinder	*cylinder;

	cylinder = scene->objects->object;
	return (cylinder);
}

static bool	plane(t_scene *scene, t_cylinder *cylinder, t_hit_record *hit_rec, float t_max)
{
	float t;

	float a = dot(sub(hit_rec->p, cylinder->transform.position), cylinder->transform.orientation);
	float b = dot(cylinder->transform.orientation, cylinder->transform.orientation);
	if (b == 0.0f || (a < 0.0f && b < 0.0f) || (a > 0.0f && b > 0.0f))
		return (false);
	t = -a / b;
	if (t < scene->t_min || t > t_max)
		return (false);
	hit_rec->t = t;
	hit_rec->normal = set((hit_rec->p.x
				- cylinder->transform.position.x) / cylinder->radius,
			(hit_rec->p.y - cylinder->transform.position.y)
			/ (cylinder->height / 2.0f), (hit_rec->p.z
				- cylinder->transform.position.z) / cylinder->radius);
	hit_rec->color = cylinder->color;
	return (true);
}

static bool	intersect_cylinder(t_scene *scene, t_ray *ray, t_hit_record *hit_rec, float t, float t_max)
{
	t_hit_record	hit_rec2;
	t_cylinder		*cylinder;

	cylinder = get_cylinder(scene);
	hit_rec2.p = sum(ray->origin, mul(ray->direction, t));
	if (plane(scene, cylinder, &hit_rec2, t_max))
		if (hit_rec2.t <= cylinder->height / 2.0f)
		{
			hit_rec->t = hit_rec2.t;
			hit_rec->normal = hit_rec2.normal;
			hit_rec->p = hit_rec2.p;
			hit_rec->color = divide_color(sum_colors(hit_rec->color, hit_rec2.color), 2);
			return (true);
		}
	cylinder->transform.orientation = mul(cylinder->transform.orientation, -1);
	if (plane(scene, cylinder, &hit_rec2, t_max))
		if (hit_rec2.t <= cylinder->height / 2.0f)
		{
			cylinder->transform.orientation = mul(cylinder->transform.orientation, -1);
			hit_rec->t = hit_rec2.t;
			hit_rec->normal = hit_rec2.normal;
			hit_rec->p = hit_rec2.p;
			hit_rec->color = divide_color(sum_colors(hit_rec->color, hit_rec2.color), 2);
			return (true);
		}
	cylinder->transform.orientation = mul(cylinder->transform.orientation, -1);
	return (false);
}

bool	hit_cylinder(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_vec3	cross1;
	t_vec3	cross2;
	float	d;
	float	tmp;

	cross1 = cross(ray->direction, get_cylinder(scene)->transform.orientation);
	t_vec3 sub1 = sub(ray->origin, get_cylinder(scene)->transform.position);
	cross2 = cross(sub1, get_cylinder(scene)->transform.orientation);
	float a = dot(cross1, cross1);
	float b = dot(cross1, cross2);
	float c = dot(cross2, cross2) - (get_cylinder(scene)->radius * get_cylinder(scene)->radius
					* dot(get_cylinder(scene)->transform.orientation,
				get_cylinder(scene)->transform.orientation));
	tmp = b * b - a * c;
	if (tmp > 0)
	{
		d = (-b - sqrt(tmp)) / a;
		if (d < t_max && d > scene->t_min)
			return (intersect_cylinder(scene, ray, hit_rec, d, t_max));
		d = (-b + sqrt(tmp)) / a;
		if (d < t_max && d > scene->t_min)
			return (intersect_cylinder(scene, ray, hit_rec, d, t_max));
	}
	return (false);
}
