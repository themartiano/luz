/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 10:41:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 18:55:08 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

int	exit_error(char *message)
{
	ft_putstr(COLOR_RED "Error\n" COLOR_NC);
	ft_putstr(COLOR_LIGHT_RED);
	ft_putstr(message);
	ft_putstr(COLOR_NC);
	exit(1);
}

int	clean_exit(t_scene *scene)
{
	while (scene->objects->next != NULL)
	{
		free(scene->objects->object);
		free(scene->objects->prev);
		scene->objects = scene->objects->next;
	}
	free(scene->objects->prev);
	exit(0);
}
