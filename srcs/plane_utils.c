/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   plane_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 17:14:24 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 18:08:40 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_plane	*get_plane(t_scene scene)
{
	t_plane	*plane;

	plane = scene.objects->object;
	return (plane);
}

static bool	update_hit_record(t_hit_record *hit_rec, t_scene scene, t_ray *ray,
float d)
{
	t_sphere	*sphere;

	sphere = get_sphere(scene);
	hit_rec->t = d;
	hit_rec->p.x = ray->origin.x + hit_rec->t * ray->direction.x;
	hit_rec->p.y = ray->origin.y + hit_rec->t * ray->direction.y;
	hit_rec->p.z = ray->origin.z + hit_rec->t * ray->direction.z;
	hit_rec->normal = set((hit_rec->p.x
				- sphere->transform.position.x) / sphere->radius,
			(hit_rec->p.y - sphere->transform.position.y)
			/ sphere->radius, (hit_rec->p.z
				- sphere->transform.position.z) / sphere->radius);
	return (true);
}

bool	hit_plane(t_scene scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	if (ray->direction.x < get_plane(scene)->transform.orientation.x)
	{
		return (update_hit_record(hit_rec, scene, ray, 1.0f));
	}
	(void)t_max;
	return (false);
}
