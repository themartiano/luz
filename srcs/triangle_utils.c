/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   triangle_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 09:59:05 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/11 18:08:56 by ejuliao-         ###   ########.fr       */
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

bool	hit_triangle(t_scene *scene, t_ray *ray, t_hit_record *hit_rec, float t_max)
{
	t_triangle	*triangle;

	triangle = get_triangle(scene);
	t_vec3 v1 = sub(triangle->p2, triangle->p1);
	t_vec3 v2 = sub(triangle->p3, triangle->p1);
	t_vec3 p = cross(ray->direction, v2);
	float d = dot(v1, p);
	if (fabs(d) < scene->t_min)
		return (false);
	float c = 1.0f / d;
	t_vec3 t = sub(ray->origin, triangle->p1);
	float a = dot(t, p) * c;
	if (a < 0.0f || a > 1.0f || fabs(t.z) > t_max)
		return (false);
	t = cross(t, v1);
	float b = dot(ray->direction, t) * c;
	if (b < 0.0f || a + b > 1.0f)
		return (false);
	hit_rec->t = dot(v2, t) * c;
	hit_rec->p = sum(ray->origin, mul(ray->direction, hit_rec->t));
	hit_rec->normal.x = (v1.y * v2.z) - (v1.z * v2.y);
	hit_rec->normal.y = (v1.z * v2.x) - (v1.x * v2.z);
	hit_rec->normal.z = (v1.x * v2.y) - (v1.y * v2.x);
	hit_rec->color = divide_color(sum_colors(hit_rec->color, triangle->color), 2);
	calc_lights(scene, hit_rec);
	return (true);
}
