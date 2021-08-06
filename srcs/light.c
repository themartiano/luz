/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   light.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/05 11:34:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/08/06 19:37:16 by ejuliao-         ###   ########.fr       */
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

static bool	object_in_shadow(t_scene scene, t_light light, t_hit_record hit_rec)
{
	t_ray	ray;

	while (scene.objects->prev != NULL)
		scene.objects = scene.objects->prev;
	ray.origin = sum(hit_rec.p, mul(hit_rec.normal, scene.epsilon));
	ray.direction = normalize(sub(light.transform.position, ray.origin));
	scene.should_calculate_light = false;
	return (check_ray_hits(&scene, ray, &hit_rec));
}

static void	compute_light(t_scene *scene, t_light *light, t_hit_record *hit_rec)
{
	t_vec3	light_n;
	float	r2;
	float	l_gain;

	if (!object_in_shadow(*scene, *light, *hit_rec))
	{
		light_n = sub(light->transform.position, hit_rec->p);
		r2 = length_sqrt(light_n);
		l_gain = dot(normalize(light_n), hit_rec->normal);
		if (l_gain <= 0.0f)
			hit_rec->l_brightness = 0.0f;
		else
			hit_rec->l_brightness = (light->brightness * l_gain * 1000.0f) / (M_PI * r2);
		hit_rec->color = sum_colors(hit_rec->color, mul_color(light->color, hit_rec->l_brightness));
	}
}

void	calc_lights(t_scene *scene, t_hit_record *hit_rec)
{
	t_light	*light;

	if (scene->lights != NULL)
	{
		while (true)
		{
			light = scene->lights->object;
			compute_light(scene, light, hit_rec);
			if (scene->lights->next == NULL)
				break ;
			scene->lights = scene->lights->next;
		}
		while (scene->lights->prev != NULL)
			scene->lights = scene->lights->prev;
	}
}
