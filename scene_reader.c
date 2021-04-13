/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/12 11:04:08 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_xyz	parse_xyz(char *str)
{
	t_xyz	values;
	char 	**input;

	input = ft_split(str, ',');
	values.x = ft_atoi(input[0]);
	values.y = ft_atoi(input[1]);
	values.z = ft_atoi(input[2]);
	return (values);
}

bool 	read_rac(char **values, t_holder *holder)
{
	t_xyz	parsed;

	if (ft_memcmp(values[0], "R", 1) == 0)
	{
		holder->scene.x_res = ft_atoi(values[1]);
		holder->scene.y_res = ft_atoi(values[2]);
		return (true);
	}
	else if (ft_memcmp(values[0], "A", 1) == 0)
	{
		holder->scene.ambient_clr.brightness = ft_atoi(values[1]);
		parsed = parse_xyz(values[2]);
		holder->scene.ambient_clr.color.r = parsed.x;
		holder->scene.ambient_clr.color.g = parsed.y;
		holder->scene.ambient_clr.color.b = parsed.z;
		return (true);
	}
	else if (ft_memcmp(values[0], "c", 1) == 0)
	{
		holder->scene.camera.transform.position = parse_xyz(values[1]);
		holder->scene.camera.transform.rotation = parse_xyz(values[2]);
		holder->scene.camera.fov = ft_atoi(values[3]);
		return (true);
	}
	return (false);
}

bool	read_l(char **values, t_holder *holder)
{
	t_xyz	parsed;

	if (ft_memcmp(values[0], "l", 1) == 0)
	{
		holder->scene.lights.light.transform.position = parse_xyz
				(values[1]);
		holder->scene.lights.light.brightness = ft_atoi(values[2]);
		parsed = parse_xyz(values[3]);
		holder->scene.lights.light.color.r = parsed.x;
		holder->scene.lights.light.color.g = parsed.y;
		holder->scene.lights.light.color.b = parsed.z;
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
	while (rv == 1)
	{
		rv = get_next_line(fd, &line);
		if (rv != 0)
		{
			values = ft_split(line, ' ');
			read_rac(values, holder);
			read_l(values, holder);
		}
		free(line);
	}
}
