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
#include "readers.h"

bool 	read_rac(char **values, t_holder *holder)
{
	if (ft_memcmp(values[0], "R", 1) == 0)
	{
		holder->scene.x_res = ft_atoi(values[1]);
		holder->scene.y_res = ft_atoi(values[2]);
		return (true);
	}
	else if (ft_memcmp(values[0], "A", 1) == 0)
	{
		holder->scene.ambient_clr.brightness = ft_atoi(values[1]);
		holder->scene.ambient_clr.color = xyz_to_rgb(parse_xyz(values[2]));
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

bool	read_lsp(char **values, t_holder *holder)
{
	t_sphere	*sphere;
	t_objects	*tmp_objects;

	if (ft_memcmp(values[0], "l", 1) == 0)
	{
		holder->scene.lights.light.transform.position = parse_xyz
			(values[1]);
		holder->scene.lights.light.brightness = ft_atoi(values[2]);
		holder->scene.lights.light.color = xyz_to_rgb(parse_xyz(values[3]));
		return (true);
	}
	else if (ft_memcmp(values[0], "sp", 2) == 0)
	{
		sphere = (t_sphere *)malloc(sizeof(*sphere));
		sphere->transform.position = parse_xyz(values[1]);
		sphere->diameter = ft_atoi(values[2]);
		sphere->color = xyz_to_rgb(parse_xyz(values[3]));
		tmp_objects = allc_end(holder);
		tmp_objects->object = sphere;
		free(sphere);
		tmp_objects->type = "sp";
		tmp_objects->next = NULL;
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
	holder->scene.objects.object = NULL;
	while (rv == 1)
	{
		rv = get_next_line(fd, &line);
		if (rv != 0)
		{
			values = ft_split(line, ' ');
			read_rac(values, holder);
			read_lsp(values, holder);
			read_pl(values, holder);
			read_sq(values, holder);
			read_cy(values, holder);
			read_tr(values, holder);
		}
		free(line);
	}
}
