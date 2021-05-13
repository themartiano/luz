/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cylinder_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 09:46:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/13 10:41:16 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_cylinder	*get_cylinder(t_scene *scene)
{
	t_cylinder	*cylinder;

	cylinder = NULL;
	if (scene->objects != NULL)
	{
		cylinder = scene->objects->object;
	}
	return (cylinder);
}

static bool	plane(t_scene *scene, t_cylinder *cylinder, t_hit_record *hit_rec2,
float t_max)
{
	float	t;
	float	a;
	float	b;

	a = dot(sub(hit_rec2->p, cylinder->transform.position),
			cylinder->transform.orientation);
	b = dot(cylinder->transform.orientation, cylinder->transform.orientation);
	if (b == 0.0f || (a < 0.0f && b < 0.0f) || (a > 0.0f && b > 0.0f))
		return (false);
	t = -a / b;
	if (t < t_max && t > scene->t_min)
	{
		hit_rec2->t = t;
		hit_rec2->normal = normalize(set(hit_rec2->p.x, hit_rec2->p.y,
					hit_rec2->p.z));
		hit_rec2->color = cylinder->color;
		return (true);
	}
	return (false);
}

static bool	update_hit_record(t_hit_record *hit_rec, t_hit_record *hit_rec2)
{
	hit_rec->t = hit_rec2->t;
	hit_rec->p = hit_rec2->p;
	hit_rec->normal = hit_rec2->normal;
	hit_rec->hit_color = hit_rec2->color;
	return (true);
}

static bool	intersect_cylinder(t_scene *scene, t_hit_record *hit_rec,
t_vec3 ray_p, float t_max)
{
	t_hit_record	hit_rec2;
	t_cylinder		*cylinder;

	cylinder = get_cylinder(scene);
	hit_rec2.p = ray_p;
	if (plane(scene, cylinder, &hit_rec2, t_max))
		if (hit_rec2.t <= cylinder->height / 2.0f)
			return (update_hit_record(hit_rec, &hit_rec2));
	cylinder->transform.orientation
		= mul(cylinder->transform.orientation, -1.0f);
	if (plane(scene, cylinder, &hit_rec2, t_max))
	{
		if (hit_rec2.t <= cylinder->height / 2.0f)
		{
			cylinder->transform.orientation
				= mul(cylinder->transform.orientation, -1.0f);
			return (update_hit_record(hit_rec, &hit_rec2));
		}
	}
	cylinder->transform.orientation
		= mul(cylinder->transform.orientation, -1.0f);
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
	cross2 = cross(sub(ray->origin, get_cylinder(scene)->transform.position),
			get_cylinder(scene)->transform.orientation);
	tmp = dot(cross1, cross2) * dot(cross1, cross2) - dot(cross1, cross1)
		* (dot(cross2, cross2) - (get_cylinder(scene)->radius
				* get_cylinder(scene)->radius
				* dot(get_cylinder(scene)->transform.orientation,
					get_cylinder(scene)->transform.orientation)));
	if (tmp > 0.0f)
	{
		d = (-dot(cross1, cross2) - sqrt(tmp)) / dot(cross1, cross1);
		if (d < t_max && d > scene->t_min)
			return (intersect_cylinder(scene, hit_rec, sum(ray->origin, mul(
							ray->direction, d)), t_max));
		d = (-dot(cross1, cross2) + sqrt(tmp)) / dot(cross1, cross1);
		if (d < t_max && d > scene->t_min)
			return (intersect_cylinder(scene, hit_rec, sum(ray->origin, mul(
							ray->direction, d)), t_max));
	}
	return (false);
}
