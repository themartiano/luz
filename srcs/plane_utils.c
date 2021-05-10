/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   plane_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 17:14:24 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/10 16:17:19 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_plane	*get_plane(t_scene *scene)
{
	t_plane	*plane;

	plane = scene->objects->object;
	return (plane);
}

bool	intersect_plane(t_plane *plane, t_ray *ray, t_hit_record *hit_rec, float t_max, float t_min)
{
	float	d;
	float	t;

	d = dot(ray->direction, plane->transform.orientation);
	if (!d)
		return (false);
	t = dot(sub(plane->transform.position, ray->origin),
			plane->transform.orientation) / d;
	if (t < t_max && t > t_min)
	{
		hit_rec->t = t;
		hit_rec->p = sum(ray->origin, mul(ray->direction, hit_rec->t));
		if (dot(ray->direction, plane->transform.orientation) < 0.0f)
			hit_rec->normal = mul(normalize(hit_rec->p), -1.0f);
		else
			hit_rec->normal = normalize(hit_rec->p);
		hit_rec->color = divide_color(
				sum_colors(hit_rec->color, plane->color), 2);
		return (true);
	}
	return (false);
}

bool	hit_plane(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_plane	*plane;

	plane = get_plane(scene);
	if (intersect_plane(plane, ray, hit_rec, t_max, scene->t_min))
	{
		calc_lights(scene, hit_rec);
		return (true);
	}
	return (false);
}
