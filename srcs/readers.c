/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:13:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/13 13:13:03 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_objects 	*allc_end(t_holder *holder)
{
	t_objects	*tmp_objects;

	tmp_objects = &holder->scene.objects;
	while (tmp_objects->next != NULL)
		tmp_objects = tmp_objects->next;
	tmp_objects->next = (t_objects *) malloc(sizeof(*tmp_objects));
	tmp_objects = tmp_objects->next;
	return (tmp_objects);
}

bool	read_pl(char **values, t_holder *holder)
{
	t_plane		*plane;
	t_objects	*tmp_objects;

	if (ft_memcmp(values[0], "pl", 2) == 0)
	{
		plane = (t_plane *)malloc(sizeof(*plane));
		plane->transform.position = parse_xyz(values[1]);
		plane->transform.rotation = parse_xyz(values[2]);
		plane->color = vec3_to_rgb(parse_xyz(values[3]));
		tmp_objects = allc_end(holder);
		tmp_objects->object = plane;
		free(plane);
		tmp_objects->type = "pl";
		tmp_objects->next = NULL;
		return (true);
	}
	return (false);
}

bool	read_sq(char **values, t_holder *holder)
{
	t_square	*square;
	t_objects	*tmp_objects;

	if (ft_memcmp(values[0], "sq", 2) == 0)
	{
		square = (t_square *)malloc(sizeof(*square));
		square->transform.position = parse_xyz(values[1]);
		square->transform.rotation = parse_xyz(values[2]);
		square->side_size = ft_atoi(values[3]);
		square->color = vec3_to_rgb(parse_xyz(values[4]));
		tmp_objects = allc_end(holder);
		tmp_objects->object = square;
		free(square);
		tmp_objects->type = "sq";
		tmp_objects->next = NULL;
		return (true);
	}
	return (false);
}

bool	read_cy(char **values, t_holder *holder)
{
	t_cylinder	*cylinder;
	t_objects	*tmp_objects;

	if (ft_memcmp(values[0], "cy", 2) == 0)
	{
		cylinder = (t_cylinder *)malloc(sizeof(*cylinder));
		cylinder->transform.position = parse_xyz(values[1]);
		cylinder->transform.rotation = parse_xyz(values[2]);
		cylinder->diameter = ft_atoi(values[3]);
		cylinder->height = ft_atoi(values[4]);
		cylinder->color = vec3_to_rgb(parse_xyz(values[5]));
		tmp_objects = allc_end(holder);
		tmp_objects->object = cylinder;
		free(cylinder);
		tmp_objects->type = "cy";
		tmp_objects->next = NULL;
		return (true);
	}
	return (false);
}

bool	read_tr(char **values, t_holder *holder)
{
	t_triangle	*triangle;
	t_objects	*tmp_objects;

	if (ft_memcmp(values[0], "tr", 2) == 0)
	{
		triangle = (t_triangle *)malloc(sizeof(*triangle));
		triangle->p1 = parse_xyz(values[1]);
		triangle->p2 = parse_xyz(values[2]);
		triangle->p3 = parse_xyz(values[3]);
		triangle->color = vec3_to_rgb(parse_xyz(values[4]));
		tmp_objects = allc_end(holder);
		tmp_objects->object = triangle;
		free(triangle);
		tmp_objects->type = "tr";
		tmp_objects->next = NULL;
		return (true);
	}
	return (false);
}
