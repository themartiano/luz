/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/11 15:18:05 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/11 16:11:42 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_camera	*get_camera(t_scene *scene)
{
	t_camera	*camera;

	camera = NULL;
	if (scene->cameras != NULL)
	{
		camera = scene->cameras->object;
	}
	return (camera);
}

void	store_camera(t_scene *scene, t_object *object)
{
	object->next = NULL;
	object->prev = NULL;
	if (scene->cameras == NULL)
		scene->cameras = object;
	else
	{
		while (scene->cameras->next != NULL)
			scene->cameras = scene->cameras->next;
		object->prev = scene->cameras;
		scene->cameras->next = object;
		while (scene->cameras->prev != NULL)
			scene->cameras = scene->cameras->prev;
	}
}