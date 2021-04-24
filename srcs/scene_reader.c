/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 19:03:07 by ejuliao-         ###   ########.fr       */
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

bool	read_ra(char **values, t_holder *holder)
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
	return (false);
}

bool	read_c(char **values, t_holder *holder)
{
	float	theta;

	if (ft_memcmp(values[0], "c", 1) == 0)
	{
		holder->scene.camera.transform.position = parse_xyz(values[1]);
		holder->scene.camera.transform.orientation = parse_xyz(values[2]);
		holder->scene.camera.fov = ft_atoi(values[3]);
		theta = holder->scene.camera.fov * M_PI / 180;
		holder->scene.camera.half_width = tan(theta / 2);
		holder->scene.camera.half_height = ((float)holder->scene.y_res
				/ (float)holder->scene.x_res)
			* holder->scene.camera.half_width;
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
		if (read_ra(values, holder) || read_c(values, holder)
			|| read_l(values, holder) || read_sp(values, holder)
			|| read_sq(values, holder) || read_cy(values, holder))
			continue ;
		read_tr(values, holder);
	}
	free(line);
}
