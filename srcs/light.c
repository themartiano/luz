/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   light.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/05 11:34:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/05 13:06:05 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	store_light(t_scene *scene, t_object *object)
{
	object->next = NULL;
	object->prev = NULL;
	if (scene->lights == NULL)
		scene->lights = object;
	else
	{
		while (scene->lights->next != NULL)
			scene->lights = scene->lights->next;
		object->prev = scene->lights;
		scene->lights->next = object;
		while (scene->lights->prev != NULL)
			scene->lights = scene->lights->prev;
	}
}

void	average_w_light_clr(t_scene *scene, t_ray *ray, t_hit_record *hit_rec)
{
	t_light	*light;

	if (scene->lights != NULL && scene->lights->type == 5)
	{
		light = scene->lights->object;
		hit_rec->color = sum_colors(hit_rec->color, mul_color(light->color, light->brightness));
	}
}
