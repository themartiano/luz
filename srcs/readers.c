/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:13:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 19:03:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

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

bool	read_pl(char **values, t_holder *holder)
{
	t_plane		*plane;
	t_object	*object;

	if (ft_memcmp(values[0], "pl", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		plane = (t_plane *)malloc(sizeof(*plane));
		plane = (t_plane *)malloc(sizeof(*plane));
		plane->transform.position = parse_xyz(values[1]);
		plane->transform.orientation = parse_xyz(values[2]);
		plane->color = vec3_to_rgb(parse_xyz(values[3]));
		object->object = plane;
		object->type = 1;
		store_object(holder, object);
		return (true);
	}
	return (false);
}

bool	read_sq(char **values, t_holder *holder)
{
	t_square	*square;
	t_object	*object;

	if (ft_memcmp(values[0], "sq", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		square = (t_square *)malloc(sizeof(*square));
		square->transform.position = parse_xyz(values[1]);
		square->transform.orientation = parse_xyz(values[2]);
		square->side_size = ft_atoi(values[3]);
		square->color = vec3_to_rgb(parse_xyz(values[4]));
		object->object = square;
		object->type = 2;
		store_object(holder, object);
		return (true);
	}
	return (false);
}

bool	read_cy(char **values, t_holder *holder)
{
	t_cylinder	*cylinder;
	t_object	*object;

	if (ft_memcmp(values[0], "cy", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		cylinder = (t_cylinder *)malloc(sizeof(*cylinder));
		cylinder->transform.position = parse_xyz(values[1]);
		cylinder->transform.orientation = parse_xyz(values[2]);
		cylinder->radius = ft_atoi(values[3]) / 2;
		cylinder->height = ft_atoi(values[4]);
		cylinder->color = vec3_to_rgb(parse_xyz(values[5]));
		object->object = cylinder;
		object->type = 3;
		store_object(holder, object);
		return (true);
	}
	return (false);
}

bool	read_tr(char **values, t_holder *holder)
{
	t_triangle	*triangle;
	t_object	*object;

	if (ft_memcmp(values[0], "tr", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		triangle = (t_triangle *)malloc(sizeof(*triangle));
		triangle->p1 = parse_xyz(values[1]);
		triangle->p2 = parse_xyz(values[2]);
		triangle->p3 = parse_xyz(values[3]);
		triangle->color = vec3_to_rgb(parse_xyz(values[4]));
		object->object = triangle;
		object->type = 4;
		store_object(holder, object);
		return (true);
	}
	return (false);
}
