/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   triangle_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 09:59:05 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 10:15:24 by ejuliao-         ###   ########.fr       */
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
	float	a;

	v1 = sub(get_triangle(scene)->p2, get_triangle(scene)->p1);
	v2 = sub(get_triangle(scene)->p3, get_triangle(scene)->p1);
	d = dot(v1, cross(ray->direction, v2));
	if (fabs(d) < scene->t_min)
		return (false);
	t = sub(ray->origin, get_triangle(scene)->p1);
	a = dot(t, cross(ray->direction, v2)) * (1.0f / d);
	if (a < 0.0f || a > 1.0f || fabs(t.z) > t_max)
		return (false);
	t = cross(t, v1);
	if (dot(ray->direction, t) * (1.0f / d) < 0.0f || a
		+ ((dot(ray->direction, t) * (1.0f / d))) > 1.0f)
		return (false);
	hit_rec->t = dot(v2, t) * (1.0f / d);
	hit_rec->p = sum(mul(v1, a), mul(v2, dot(ray->direction, t) * (1.0f / d)));
	hit_rec->normal = cross(v1, v2);
	//hit_rec->normal = normalize(hit_rec->p);
	//hit_rec->normal = mul(normalize(hit_rec->normal), -1.0f);
	//printf("%f, %f, %f\n", hit_rec->normal.x, hit_rec->normal.y, hit_rec->normal.z);
	hit_rec->hit_color = get_triangle(scene)->color;
	return (true);
}
