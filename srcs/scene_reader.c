/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 20:02:43 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"
#include "readers.h"

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

bool	read_ra(char **values, t_scene *scene)
{
	if (ft_memcmp(values[0], "R", 1) == 0)
	{
		scene->x_res = ft_atoi(values[1]);
		scene->y_res = ft_atoi(values[2]);
		return (true);
	}
	else if (ft_memcmp(values[0], "A", 1) == 0)
	{
		scene->amb_light.brightness = ft_atof(values[1]) / 6.4f;
		scene->amb_light.color = vec3_to_rgb(parse_xyz(values[2]));
		return (true);
	}
	return (false);
}

bool	read_c(char **values, t_scene *scene)
{
	float	theta;

	if (ft_memcmp(values[0], "c", 1) == 0)
	{
		scene->camera.transform.position = parse_xyz(values[1]);
		scene->camera.transform.orientation = parse_xyz(values[2]);
		if (scene->camera.transform.orientation.z == 0)
			scene->camera.transform.orientation.z = -1.0f;
		scene->camera.transform.orientation.y *= -1;
		scene->camera.fov = ft_atoi(values[3]);
		theta = scene->camera.fov * M_PI / 180;
		scene->camera.half_width = tan(theta / 2);
		scene->camera.half_height = ((float)scene->y_res
				/ (float)scene->x_res)
			* scene->camera.half_width;
		return (true);
	}
	return (false);
}

bool	read_l(char **values, t_scene *scene)
{
	t_light		*light;
	t_object	*object;

	if (ft_memcmp(values[0], "l", 1) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		light = (t_light *)malloc(sizeof(*light));
		light->transform.position = parse_xyz (values[1]);
		light->brightness = ft_atof(values[2]);
		light->color = vec3_to_rgb(parse_xyz(values[3]));
		object->object = light;
		object->type = 5;
		store_object(scene, object);
		return (true);
	}
	return (false);
}

void	read_scene(int fd, t_holder *holder)
{
	char	*line;
	char	**values;
	int		rv;

	rv = 1;
	line = NULL;
	while (rv == 1)
	{
		free(line);
		rv = get_next_line(fd, &line);
		values = ft_split(line, ' ');
		if (read_ra(values, &holder->scene) || read_c(values, &holder->scene)
			|| read_l(values, &holder->scene) || read_sp(values, &holder->scene)
			|| read_sq(values, &holder->scene)
			|| read_cy(values, &holder->scene)
			|| read_tr(values, &holder->scene))
			continue ;
	}
	free(line);
}
