/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   square_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 10:36:58 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/11 16:11:18 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_square	*get_square(t_scene *scene)
{
	t_square	*square;

	square = NULL;
	if (scene->objects != NULL)
	{
		square = scene->objects->object;
	}
	return (square);
}

bool	hit_square(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_square	*square;

	square = get_square(scene);
	float a = dot(sub(ray->origin, square->transform.position),
			square->transform.orientation);
	float b = dot(ray->direction, square->transform.orientation);
	if (b == 0.0f || (b < 0.0f && a < 0.0f) || (b > 0.0f && a > 0.0f))
		return (false);
	float t = -a / b;
	t_vec3 d = sub(sum(mul(ray->direction, t), ray->origin),
			square->transform.position);
	if (fabs(d.x) > square->half_side_size || fabs(d.y) > square->half_side_size
		|| fabs(d.z) > square->half_side_size)
		return (false);
	if (t > 0.0f)
	{
		hit_rec->t = t;
		hit_rec->color = divide_color(sum_colors(hit_rec->color, square->color), 2);
		return (true);
	}
	return (false);
	(void)t_max;
}
