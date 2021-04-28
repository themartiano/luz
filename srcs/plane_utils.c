/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   plane_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 17:14:24 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/28 10:57:22 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_plane	*get_plane(t_scene *scene)
{
	t_plane	*plane;

	plane = scene->objects->object;
	return (plane);
}

bool	hit_plane(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_plane	*plane;
	float	d;
	float	t;

	plane = get_plane(scene);
	d = dot(ray->direction, plane->transform.position);
	if (!d)
		return (false);
	t = dot(sub(plane->transform.orientation, ray->origin), ray->direction) / d;
	if (t < t_max && t > scene->t_min)
	{
		hit_rec->t = t;
		hit_rec->p.x = ray->origin.x + hit_rec->t * ray->direction.x;
		hit_rec->p.y = ray->origin.y + hit_rec->t * ray->direction.y;
		hit_rec->p.z = ray->origin.z + hit_rec->t * ray->direction.z;
		if (dot(ray->direction, plane->transform.orientation) > 0)
			hit_rec->normal = scale(plane->transform.orientation, -1.0f);
		else
			hit_rec->normal = plane->transform.orientation;
		return (true);
	}
	return (false);
}
