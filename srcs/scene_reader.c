/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/19 14:46:34 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"
#include "readers.h"

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
		holder->scene.ambient_clr.brightness = ft_atoi(values[1]);
		holder->scene.ambient_clr.color = vec3_to_rgb(parse_xyz(values[2]));
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

bool	read_lsp(char **values, t_holder *holder)
{
	t_sphere	*sphere;

	if (ft_memcmp(values[0], "l", 1) == 0)
	{
		holder->scene.light.transform.position = parse_xyz
			(values[1]);
		holder->scene.light.brightness = ft_atoi(values[2]);
		holder->scene.light.color = vec3_to_rgb(parse_xyz(values[3]));
		return (true);
	}
	else if (ft_memcmp(values[0], "sp", 2) == 0)
	{
		sphere = (t_sphere *)malloc(sizeof(*sphere));
		sphere->prev = NULL;
		sphere->next = NULL;
		sphere->transform.position = parse_xyz(values[1]);
		sphere->radius = ft_atoi(values[2]) / 2;
		sphere->color = vec3_to_rgb(parse_xyz(values[3]));
		if (holder->scene.sphere == NULL)
			holder->scene.sphere = sphere;
		else
		{
			while (holder->scene.sphere->next != NULL)
				holder->scene.sphere = holder->scene.sphere->next;
			sphere->prev = holder->scene.sphere;
			holder->scene.sphere->next = sphere;
			while (holder->scene.sphere->prev != NULL)
				holder->scene.sphere = holder->scene.sphere->prev;
		}
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
			read_lsp(values, holder);
			read_pl(values, holder);
			read_sq(values, holder);
			read_cy(values, holder);
			read_tr(values, holder);
		}
		free(line);
	}
}
