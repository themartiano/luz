/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   light.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/05 11:34:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/05 16:29:08 by ejuliao-         ###   ########.fr       */
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

// static float	manage_hit(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
// {
// 	hit_rec->hit = true;
// 	gen_pixel_clr(scene, ray, &hit_rec->color, hit_rec->t);
// 	return (hit_rec->t);
// }

// static bool	check_l_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
// {
// 	float	closest;

// 	hit_rec->hit = false;
// 	closest = scene->t_max;
// 	while (true)
// 	{
// 		if (scene->objects != NULL && ((scene->objects->type == 0
// 					&& hit_sphere(scene, &ray, hit_rec, closest))
// 				|| (scene->objects->type == 1
// 					&& hit_plane(scene, &ray, hit_rec, closest))
// 				|| (scene->objects->type == 3
// 					&& hit_cylinder(scene, &ray, hit_rec, closest))))
// 			closest = manage_hit(scene, ray, hit_rec);
// 		if (scene->objects->next == NULL)
// 			break ;
// 		else
// 			scene->objects = scene->objects->next;
// 	}
// 	while (scene->objects->prev != NULL)
// 		scene->objects = scene->objects->prev;
// 	return (hit_rec->hit);
// }

void	average_w_light_clr(t_scene *scene, t_ray *ray, t_hit_record *hit_rec)
{
	t_light		*light;
	static bool	calling = false;
	t_hit_record	new_hr;

	if (calling == false && scene->lights != NULL && scene->lights->type == 5)
	{
		calling = true;
		new_hr.color = set_color(255, 255, 255);
		light = scene->lights->object;
		if (check_ray_hits(scene, gen_ray(scene, scene->crrnt_pxl,
				scene->camera.transform.position,
				scene->camera.transform.orientation),
			&new_hr))
		{
			printf("HIT\n");
			hit_rec->color = sum_colors(hit_rec->color, mul_color(light->color, light->brightness));
		}
		calling = false;
	}
}
