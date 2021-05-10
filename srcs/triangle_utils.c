/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   triangle_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 09:59:05 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/10 10:17:17 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_triangle	*get_triangle(t_scene *scene)
{
	t_triangle	*triangle;

	triangle = scene->objects->object;
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
	if (a < 0.0f || a > t_max)
		return (false);
	t = cross(t, v1);
	float b = dot(ray->direction, t) * c;
	if (b < 0.0f || a + b > t_max)
		return (false);
	hit_rec->t = dot(v2, t) * c;
	hit_rec->color = divide_color(sum_colors(hit_rec->color, triangle->color), 2);
	return (true);
}
