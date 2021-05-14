/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/11 15:18:05 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 18:36:47 by ejuliao-         ###   ########.fr       */
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

void	change_camera(t_scene *scene)
{
	if (scene->cameras != NULL)
	{
		if (scene->cameras->next != NULL || scene->cameras->prev != NULL)
		{
			printf(COLOR_LIGHT_GRAY"\n\nChanging camera...\n"COLOR_NC);
			clear_rendering_thread(scene);
		}
		if (scene->cameras->next != NULL)
			scene->cameras = scene->cameras->next;
		else if (scene->cameras->prev != NULL)
			scene->cameras = scene->cameras->prev;
	}
}

void	move_camera(t_scene *scene, int keycode)
{
	t_camera	*camera;

	camera = get_camera(scene);
	if (keycode == KEY_W)
		camera->transform.position.z -= CAMERA_MOVE_STEP;
	else if (keycode == KEY_A)
		camera->transform.position.x -= CAMERA_MOVE_STEP;
	else if (keycode == KEY_S)
		camera->transform.position.z += CAMERA_MOVE_STEP;
	else if (keycode == KEY_D)
		camera->transform.position.x += CAMERA_MOVE_STEP;
	else if (keycode == KEY_Q)
		camera->transform.position.y -= CAMERA_MOVE_STEP;
	else if (keycode == KEY_E)
		camera->transform.position.y += CAMERA_MOVE_STEP;
	clear_rendering_thread(scene);
}

void	rotate_camera(t_scene *scene, int keycode)
{
	t_camera	*camera;

	camera = get_camera(scene);
	if (keycode == KEY_UP)
		camera->transform.orientation.z -= CAMERA_ROTATE_STEP;
	else if (keycode == KEY_LEFT)
		camera->transform.orientation.x -= CAMERA_ROTATE_STEP;
	else if (keycode == KEY_DOWN)
		camera->transform.orientation.z += CAMERA_ROTATE_STEP;
	else if (keycode == KEY_RIGHT)
		camera->transform.orientation.x += CAMERA_ROTATE_STEP;
	clear_rendering_thread(scene);
}
