/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cylinder_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 09:46:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 10:27:45 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_cylinder	*get_cylinder(t_scene *scene)
{
	t_cylinder	*cylinder;

	cylinder = scene->objects->object;
	return (cylinder);
}

bool	hit_cylinder(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
float t_max)
{
	t_cylinder	*cylinder;
	float	d;
	float	t;

	cylinder = get_cylinder(scene);
	d = dot(ray->direction, cylinder->transform.orientation);
	if (!d)
		return (false);
	t = dot(sub(cylinder->transform.position, ray->origin),
			cylinder->transform.orientation) / d;
	if (t < t_max && t > scene->t_min)
	{
		hit_rec->t = t;
		hit_rec->p.x = ray->origin.x + hit_rec->t * ray->direction.x;
		hit_rec->p.y = ray->origin.y + hit_rec->t * ray->direction.y;
		hit_rec->p.z = ray->origin.z + hit_rec->t * ray->direction.z;
		if (dot(ray->direction, cylinder->transform.orientation) > 0)
			hit_rec->normal = scale(cylinder->transform.orientation, -1.0f);
		else
			hit_rec->normal = cylinder->transform.orientation;
		hit_rec->color = divide_color(
				sum_colors(hit_rec->color, cylinder->color), 2);
		return (true);
	}
	return (false);
}
