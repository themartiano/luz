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
		t_objects *tmp_objects = &holder->scene.objects;
		while (tmp_objects->next != NULL)
		{
			tmp_objects = tmp_objects->next;
		}
		tmp_objects->next = (t_objects *)malloc(sizeof(*tmp_objects));
		tmp_objects = tmp_objects->next;
		tmp_objects->object = sphere;
		free(sphere);
		tmp_objects->type = "sp";
		tmp_objects->next = NULL;
		return (true);
	}
	return (false);
}

bool	read_plsq(char **values, t_holder *holder)
{
	t_plane		*plane;
	t_square	*square;

	if (ft_memcmp(values[0], "pl", 2) == 0)
	{
		plane = (t_plane *)malloc(sizeof(*plane));
		plane->transform.position = parse_xyz(values[1]);
		plane->transform.rotation = parse_xyz(values[2]);
		plane->color = xyz_to_rgb(parse_xyz(values[3]));
		t_objects *tmp_objects = &holder->scene.objects;
		while (tmp_objects->next != NULL)
		{
			tmp_objects = tmp_objects->next;
		}
		tmp_objects->next = (t_objects *)malloc(sizeof(*tmp_objects));
		tmp_objects = tmp_objects->next;
		tmp_objects->object = plane;
		free(plane);
		tmp_objects->type = "pl";
		tmp_objects->next = NULL;
		return (true);
	}
	else if (ft_memcmp(values[0], "sq", 2) == 0)
	{
		square = (t_square *)malloc(sizeof(*square));
		square->transform.position = parse_xyz(values[1]);
		square->transform.rotation = parse_xyz(values[2]);
		square->side_size = ft_atoi(values[3]);
		square->color = xyz_to_rgb(parse_xyz(values[4]));
		t_objects *tmp_objects = &holder->scene.objects;
		while (tmp_objects->next != NULL)
		{
			tmp_objects = tmp_objects->next;
		}
		tmp_objects->next = (t_objects *)malloc(sizeof(*tmp_objects));
		tmp_objects = tmp_objects->next;
		tmp_objects->object = square;
		free(square);
		tmp_objects->type = "sq";
		tmp_objects->next = NULL;
		return (true);
	}
	return (false);
}

bool	read_cytr(char **values, t_holder *holder)
{
	t_cylinder	*cylinder;
	t_triangle	*triangle;

	if (ft_memcmp(values[0], "cy", 2) == 0)
	{
		cylinder = (t_cylinder *)malloc(sizeof(*cylinder));
		cylinder->transform.position = parse_xyz(values[1]);
		cylinder->transform.rotation = parse_xyz(values[2]);
		cylinder->diameter = ft_atoi(values[3]);
		cylinder->height = ft_atoi(values[4]);
		cylinder->color = xyz_to_rgb(parse_xyz(values[5]));
		t_objects *tmp_objects = &holder->scene.objects;
		while (tmp_objects->next != NULL)
		{
			tmp_objects = tmp_objects->next;
		}
		tmp_objects->next = (t_objects *)malloc(sizeof(*tmp_objects));
		tmp_objects = tmp_objects->next;
		tmp_objects->object = cylinder;
		free(cylinder);
		tmp_objects->type = "cy";
		tmp_objects->next = NULL;
		return (true);
	}
	else if (ft_memcmp(values[0], "tr", 2) == 0)
	{
		triangle = (t_triangle *)malloc(sizeof(*triangle));
		triangle->p1 = parse_xyz(values[1]);
		triangle->p2 = parse_xyz(values[2]);
		triangle->p3 = parse_xyz(values[3]);
		triangle->color = xyz_to_rgb(parse_xyz(values[4]));
		t_objects *tmp_objects = &holder->scene.objects;
		while (tmp_objects->next != NULL)
		{
			tmp_objects = tmp_objects->next;
		}
		tmp_objects->next = (t_objects *)malloc(sizeof(*tmp_objects));
		tmp_objects = tmp_objects->next;
		tmp_objects->object = triangle;
		free(triangle);
		tmp_objects->type = "tr";
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
			read_plsq(values, holder);
			read_cytr(values, holder);
		}
		free(line);
	}
}
