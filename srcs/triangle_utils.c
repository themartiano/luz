/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   triangle_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 09:59:05 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 11:51:39 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_triangle	*get_triangle(t_scene *scene)
{
	t_triangle	*triangle;

	triangle = NULL;
	if (scene->objects != NULL)
	{
		triangle = scene->objects->object;
	}
	return (triangle);
}

bool	hit_triangle(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_vec3	v1;
	t_vec3	v2;
	t_vec3	t;
	float	d;
	float	u;

	v1 = sub(get_triangle(scene)->p2, get_triangle(scene)->p1);
	v2 = sub(get_triangle(scene)->p3, get_triangle(scene)->p1);
	d = dot(v1, cross(ray->direction, v2));
	if (fabs(d) < scene->epsilon)
		return (false);
	t = sub(ray->origin, get_triangle(scene)->p1);
	u = dot(t, cross(ray->direction, v2)) * (1.0f / d);
	if (u < 0.0f || u > 1.0f || fabs(t.z) > t_max)
		return (false);
	if (dot(ray->direction, cross(t, v1)) * (1.0f / d) < 0.0f || u
		+ (dot(ray->direction, cross(t, v1)) * (1.0f / d)) > 1.0f)
		return (false);
	hit_rec->t = dot(v2, cross(t, v1)) * (1.0f / d);
	hit_rec->p = sum(get_triangle(scene)->p1, sub(mul(v1, u), mul(v2,
					dot(ray->direction, cross(t, v1)) * (1.0f / d))));
	hit_rec->normal = normalize(cross(v1, v2));
	hit_rec->hit_color = get_triangle(scene)->color;
	return (true);
}
