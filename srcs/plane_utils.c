/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   plane_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 17:14:24 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 10:30:13 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_plane	*get_plane(t_scene *scene)
{
	t_plane	*plane;

	plane = NULL;
	if (scene->objects != NULL)
	{
		plane = scene->objects->object;
	}
	return (plane);
}

bool	hit_plane(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_plane	*plane;
	float	d;
	float	t;

	plane = get_plane(scene);
	d = dot(ray->direction, plane->transform.orientation);
	if (!d)
		return (false);
	t = dot(sub(plane->transform.position, ray->origin),
			plane->transform.orientation) / d;
	if (t < t_max && t > scene->epsilon)
	{
		hit_rec->t = t;
		hit_rec->p = sum(ray->origin, mul(ray->direction, hit_rec->t));
		if (dot(ray->direction, plane->transform.orientation) < 0.0f)
			hit_rec->normal = mul(normalize(hit_rec->p), -1.0f);
		else
			hit_rec->normal = normalize(hit_rec->p);
		hit_rec->hit_color = plane->color;
		return (true);
	}
	return (false);
}
