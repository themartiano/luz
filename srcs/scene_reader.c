/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/23 19:45:27 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"
#include "readers.h"

void	store_object(t_holder *holder, t_object *object)
{
	object->next = NULL;
	object->prev = NULL;
	if (holder->scene.objects == NULL)
		holder->scene.objects = object;
	else
	{
		while (holder->scene.objects->next != NULL)
			holder->scene.objects = holder->scene.objects->next;
		object->prev = holder->scene.objects;
		holder->scene.objects->next = object;
		while (holder->scene.objects->prev != NULL)
			holder->scene.objects = holder->scene.objects->prev;
	}
}

bool	read_rac(char **values, t_holder *holder)
{
	if (ft_memcmp(values[0], "R", 1) == 0)
	{
		holder->scene.x_res = ft_atoi(values[1]);
		holder->scene.y_res = ft_atoi(values[2]);
		return (true);
	}
	else if (ft_memcmp(values[0], "A", 1) == 0)
	{
		holder->scene.amb_light.brightness = ft_atof(values[1]) / 6.4f;
		holder->scene.amb_light.color = vec3_to_rgb(parse_xyz(values[2]));
		return (true);
	}
	else if (ft_memcmp(values[0], "c", 1) == 0)
	{
		holder->scene.camera.transform.position = parse_xyz(values[1]);
		holder->scene.camera.transform.orientation = parse_xyz(values[2]);
		holder->scene.camera.fov = ft_atoi(values[3]);
		return (true);
	}
	return (false);
}

bool	read_l(char **values, t_holder *holder)
{
	if (ft_memcmp(values[0], "l", 1) == 0)
	{
		holder->scene.light.transform.position = parse_xyz (values[1]);
		holder->scene.light.brightness = ft_atof(values[2]);
		holder->scene.light.color = vec3_to_rgb(parse_xyz(values[3]));
		return (true);
	}
	return (false);
}

bool	read_sp(char **values, t_holder *holder)
{
	t_sphere	*sphere;
	t_object	*object;

	if (ft_memcmp(values[0], "sp", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		sphere = (t_sphere *)malloc(sizeof(*sphere));
		sphere->transform.position = parse_xyz(values[1]);
		sphere->radius = ft_atof(values[2]) / 2.0f;
		sphere->color = vec3_to_rgb(parse_xyz(values[3]));
		object->object = sphere;
		object->type = 0;
		store_object(holder, object);
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
		if (read_rac(values, holder))
			continue ;
		if (read_l(values, holder))
			continue ;
		if (read_sp(values, holder))
			continue ;
		if (read_pl(values, holder))
			continue ;
		if (read_sq(values, holder))
			continue ;
		if (read_cy(values, holder))
			continue ;
		read_tr(values, holder);
	}
	free(line);
}
