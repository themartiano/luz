/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sphere_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 12:38:12 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/21 18:20:28 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	update_hit_record(t_hit_record *hit_record, t_scene scene, t_ray *ray,
float a)
{
	t_vec3	sp_pos;

	sp_pos = scene.sphere->transform.position;
	hit_record->t = a;
	hit_record->p.x = ray->origin.x + hit_record->t * ray->direction.x;
	hit_record->p.y = ray->origin.y + hit_record->t * ray->direction.y;
	hit_record->p.z = ray->origin.z + hit_record->t * ray->direction.z;
	a = scene.sphere->radius;
	hit_record->normal = set((hit_record->p.x - sp_pos.x) / a,
			(hit_record->p.y - sp_pos.y) / a, (hit_record->p.z - sp_pos.z) / a);
}

bool	hit_sphere(t_scene scene, t_ray *ray, t_hit_record *hit_record,
float t_max)
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
	if (b * b - a * c > 0)
	{
		d = (-b - sqrt(b * b - a * c)) / a;
		if (d < t_max && d > scene.t_min)
		{
			update_hit_record(hit_record, scene, ray, d);
			return (true);
		}
		d = (-b + sqrt(b * b - a * c)) / a;
		if (d < t_max && d > scene.t_min)
		{
			update_hit_record(hit_record, scene, ray, d);
			return (true);
		}
	}
	return (false);
}
