/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   light.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/05 11:34:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/06 12:38:25 by ejuliao-         ###   ########.fr       */
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

static void	compute_light(t_light *light, t_hit_record *hit_rec)
{
	t_vec3	light_n;
	float	r2;
	float	l_gain;
	float	l_brightness;

	light_n = sub(light->transform.position, hit_rec->p);
	r2 = length_sqrt(light_n);
	l_gain = dot(normalize(light_n), hit_rec->normal);
	if (l_gain <= 0.0f)
		l_brightness = 0.0f;
	else
		l_brightness = (light->brightness * l_gain * 1000.0f) * (4.0f * M_PI * r2);
	hit_rec->color = sum_colors(hit_rec->color, mul_color(light->color, l_brightness));
}

void	calc_lights(t_scene *scene, t_hit_record *hit_rec)
{
	t_light	*light;

	while (true)
	{
		light = scene->lights->object;
		compute_light(light, hit_rec);
		if (scene->lights->next == NULL)
			break ;
		scene->lights = scene->lights->next;
	}
	while (scene->lights->prev != NULL)
		scene->lights = scene->lights->prev;
}
