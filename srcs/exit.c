/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 10:41:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/08/06 11:21:43 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

int	exit_error(t_scene *scene, char *message)
{
	printf(COLOR_RED "Error\n" COLOR_NC);
	printf(COLOR_LIGHT_RED "%s\n" COLOR_NC, message);
	clean_exit(scene, 1);
	return (1);
}

static void	clean_cameras(t_scene *scene)
{
	if (scene->cameras != NULL)
	{
		while (scene->cameras->prev != NULL)
			scene->cameras = scene->cameras->prev;
		while (scene->cameras->next != NULL)
		{
			if (scene->cameras->object != NULL)
				free(scene->cameras->object);
			if (scene->cameras->prev != NULL)
				free(scene->cameras->prev);
			scene->cameras = scene->cameras->next;
		}
		if (scene->cameras->prev != NULL)
			free(scene->cameras->prev);
	}
}

static void	clean_lights(t_scene *scene)
{
	if (scene->lights != NULL)
	{
		while (scene->lights->prev != NULL)
			scene->lights = scene->lights->prev;
		while (scene->lights->next != NULL)
		{
			if (scene->lights->object != NULL)
				free(scene->lights->object);
			if (scene->lights->prev != NULL)
				free(scene->lights->prev);
			scene->lights = scene->lights->next;
		}
		if (scene->lights->prev != NULL)
			free(scene->lights->prev);
	}
}

int	clean_exit(t_scene *scene, int code)
{
	printf(COLOR_CYAN "\nExiting..." COLOR_NC "\n");
	// if (scene->thread != (pthread_t) NULL)
	// 	pthread_cancel(scene->thread);
	while (scene->objects != NULL && scene->objects->prev != NULL)
		scene->objects = scene->objects->prev;
	while (scene->objects != NULL && scene->objects->next != NULL)
	{
		if (scene->objects != NULL && scene->objects->object != NULL)
			free(scene->objects->object);
		if (scene->objects != NULL && scene->objects->prev != NULL)
			free(scene->objects->prev);
		scene->objects = scene->objects->next;
	}
	if (scene->objects != NULL && scene->objects->prev != NULL)
		free(scene->objects->prev);
	clean_cameras(scene);
	clean_lights(scene);
	exit(code);
}
