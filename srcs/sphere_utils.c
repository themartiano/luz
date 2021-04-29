/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sphere_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 12:38:12 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 15:05:40 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_sphere	*get_sphere(t_scene *scene)
{
	t_sphere	*sphere;

	sphere = scene->objects->object;
	return (sphere);
}

static bool	update_hit_record(t_hit_record *hit_rec, t_scene *scene, t_ray *ray,
float d)
{
	t_sphere	*sphere;

	sphere = get_sphere(scene);
	hit_rec->t = d;
	hit_rec->p = sum(ray->origin, mul(ray->direction, hit_rec->t));
	hit_rec->normal = set((hit_rec->p.x
				- sphere->transform.position.x) / sphere->radius,
			(hit_rec->p.y - sphere->transform.position.y)
			/ sphere->radius, (hit_rec->p.z
				- sphere->transform.position.z) / sphere->radius);
	hit_rec->color = divide_color(sum_colors(hit_rec->color, sphere->color), 2);
	return (true);
}

bool	hit_sphere(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_vec3	oc;
	float	a;
	float	b;
	float	d;
	float	tmp;

	oc = sub(ray->origin, get_sphere(scene)->transform.position);
	a = dot(ray->direction, ray->direction);
	b = dot(oc, ray->direction);
	tmp = b * b - a * (dot(oc, oc) - get_sphere(scene)->radius
			* get_sphere(scene)->radius);
	if (tmp > 0)
	{
		d = (-b - sqrt(tmp)) / a;
		if (d < t_max && d > scene->t_min)
			return (update_hit_record(hit_rec, scene, ray, d));
		d = (-b + sqrt(tmp)) / a;
		if (d < t_max && d > scene->t_min)
			return (update_hit_record(hit_rec, scene, ray, d));
	}
	return (false);
}
