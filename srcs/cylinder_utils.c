/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cylinder_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 09:46:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 17:11:13 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_cylinder	*get_cylinder(t_scene *scene)
{
	t_cylinder	*cylinder;

	cylinder = scene->objects->object;
	return (cylinder);
}

static void	plane(t_cylinder *cylinder, t_hit_record *hit_rec)
{
	float t;

	float a = dot(sub(hit_rec->p, cylinder->transform.position) ,cylinder->transform.orientation);
	float b = dot(cylinder->transform.orientation, cylinder->transform.orientation);
	if (b == 0 || (a < 0 && b < 0) || (a > 0 && b > 0))
		return ;
	t = -a / b;
	if (t < 0 || hit_rec->t < t)
		return ;
	hit_rec->t = t;
	hit_rec->normal = set((hit_rec->p.x
				- cylinder->transform.position.x) / cylinder->radius,
			(hit_rec->p.y - cylinder->transform.position.y)
			/ cylinder->radius, (hit_rec->p.z
				- cylinder->transform.position.z) / cylinder->radius);
}

static bool	intersect_cylinder(t_scene *scene, t_ray *ray, t_hit_record *hit_rec, float t)
{
	t_cylinder	*cylinder;
	float		holder;

	cylinder = get_cylinder(scene);
	hit_rec->t = t;
	hit_rec->p = sum(ray->origin, mul(ray->direction, hit_rec->t));
	holder = hit_rec->t;
	plane(cylinder, hit_rec);
	if (hit_rec->t <= cylinder->height / 2.0f)
		return (true);
	cylinder->transform.orientation = mul(cylinder->transform.orientation, -1);
	plane(cylinder, hit_rec);
	if (hit_rec->t <= cylinder->height / 2.0f)
		return (true);
	hit_rec->t = holder;
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
			return (intersect_cylinder(scene, ray, hit_rec, d));
		d = (-b + sqrt(tmp)) / a;
		if (d < t_max && d > scene->t_min)
			return (intersect_cylinder(scene, ray, hit_rec, d));
	}
	return (false);
}
