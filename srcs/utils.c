/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/10 14:51:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/11 13:38:32 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	put_pixel(t_img *img, int x, int y, int color)
{
	char	*dst;

	dst = img->addr + (y * img->line_length + x
			* (img->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

char	*get_file_no_ext(const char *path)
{
	char	*name;
	char	*name_no_path;
	int		i;

	name = ft_strtrim(path, ".rt");
	i = ft_last_index_of(name, '/');
	if (i == -1)
	{
		i = ft_last_index_of(name, '\\');
		if (i == -1)
			return (name);
	}
	name_no_path = ft_substr(name, i + 1, ft_strlen(name));
	free(name);
	return (name_no_path);
}

float	length_sqrt(t_vec3 v)
{
	return (v.x * v.x + v.y * v.y + v.z * v.z);
}

float	length(t_vec3 v)
{
	return (sqrt(length_sqrt(v)));
}

void	store_object(t_scene *scene, t_object *object)
{
	object->next = NULL;
	object->prev = NULL;
	if (scene->objects == NULL)
		scene->objects = object;
	else
	{
		while (scene->objects->next != NULL)
			scene->objects = scene->objects->next;
		object->prev = scene->objects;
		scene->objects->next = object;
		while (scene->objects->prev != NULL)
			scene->objects = scene->objects->prev;
	}
}
