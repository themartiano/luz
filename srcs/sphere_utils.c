/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sphere_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 12:38:12 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 16:36:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	update_hit_record(t_hit_record *hit_rec, t_scene scene, t_ray *ray,
float d)
{
	hit_rec->t = d;
	hit_rec->p.x = ray->origin.x + hit_rec->t * ray->direction.x;
	hit_rec->p.y = ray->origin.y + hit_rec->t * ray->direction.y;
	hit_rec->p.z = ray->origin.z + hit_rec->t * ray->direction.z;
	hit_rec->normal = set((hit_rec->p.x
				- scene.sphere->transform.position.x) / scene.sphere->radius,
			(hit_rec->p.y - scene.sphere->transform.position.y)
			/ scene.sphere->radius, (hit_rec->p.z
				- scene.sphere->transform.position.z) / scene.sphere->radius);
	return (true);
}

bool	hit_sphere(t_scene scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_vec3	oc;
	float	a;
	float	b;
	float	d;
	float	tmp;

	oc.x = ray->origin.x - scene.sphere->transform.position.x;
	oc.y = ray->origin.y - scene.sphere->transform.position.y;
	oc.z = ray->origin.z - scene.sphere->transform.position.z;
	a = dot(ray->direction, ray->direction);
	b = dot(oc, ray->direction);
	tmp = b * b - a * (dot(oc, oc) - scene.sphere->radius
			* scene.sphere->radius);
	if (tmp > 0)
	{
		d = (-b - sqrt(tmp)) / a;
		if (d < t_max && d > scene.t_min)
			return (update_hit_record(hit_rec, scene, ray, d));
		d = (-b + sqrt(tmp)) / a;
		if (d < t_max && d > scene.t_min)
			return (update_hit_record(hit_rec, scene, ray, d));
	}
	return (false);
}
