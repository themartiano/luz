/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sphere_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 12:38:12 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 11:07:21 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	update_hit_record(t_hit_record *hit_record, t_scene scene, t_ray *ray,
float a)
{
	hit_record->t = a;
	hit_record->p.x = ray->origin.x + hit_record->t * ray->direction.x;
	hit_record->p.y = ray->origin.y + hit_record->t * ray->direction.y;
	hit_record->p.z = ray->origin.z + hit_record->t * ray->direction.z;
	a = scene.sphere->radius;
	hit_record->normal = set((hit_record->p.x - scene.sphere->transform.position.x) / a, (hit_record->p.y - scene.sphere->transform.position.y) / a, (hit_record->p.z - scene.sphere->transform.position.z) / a);
}

bool	hit_sphere(t_scene scene, t_ray *ray, t_hit_record *hit_record,
float t_max)
{
	t_vec3	oc;
	t_vec3	abc;
	float	d;
	float	tmp;

	oc.x = ray->origin.x - scene.sphere->transform.position.x;
	oc.y = ray->origin.y - scene.sphere->transform.position.y;
	oc.z = ray->origin.z - scene.sphere->transform.position.z;
	abc.x = dot(ray->direction, ray->direction);
	abc.y = dot(oc, ray->direction);
	abc.z = dot(oc, oc) - scene.sphere->radius * scene.sphere->radius;
	tmp = abc.y * abc.y - abc.x * abc.z;
	if (tmp > 0)
	{
		d = (-abc.y - sqrt(tmp)) / abc.x;
		if (d < t_max && d > scene.t_min)
		{
			update_hit_record(hit_record, scene, ray, d);
			return (true);
		}
		d = (-abc.y + sqrt(tmp)) / abc.x;
		if (d < t_max && d > scene.t_min)
		{
			update_hit_record(hit_record, scene, ray, d);
			return (true);
		}
	}
	return (false);
}
