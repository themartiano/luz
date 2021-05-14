/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   square_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 10:36:58 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 09:44:20 by ejuliao-         ###   ########.fr       */
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
	t_square	*sq;
	t_vec3		d;
	float		a;
	float		b;
	float		t;

	sq = get_square(scene);
	a = dot(sub(ray->origin, sq->transform.position),
			sq->transform.orientation);
	b = dot(ray->direction, sq->transform.orientation);
	if (b == 0.0f || (a < 0.0f && b < 0.0f) || (a > 0.0f && b > 0.0f))
		return (false);
	t = -a / b;
	d = sub(sum(mul(ray->direction, t), ray->origin), sq->transform.position);
	if (fabs(d.x) > sq->half_side_size || fabs(d.y) > sq->half_side_size
		|| fabs(d.z) > sq->half_side_size || t > t_max || t < scene->t_min)
		return (false);
	hit_rec->t = t;
	hit_rec->p = sum(ray->origin, mul(ray->direction, hit_rec->t));
	hit_rec->normal = normalize(hit_rec->p);
	if (dot(ray->direction, sq->transform.orientation) < 0.0f)
		hit_rec->normal = mul(normalize(hit_rec->p), -1.0f);
	hit_rec->hit_color = sq->color;
	return (true);
}
