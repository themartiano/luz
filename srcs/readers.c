/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:13:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/15 19:42:11 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	read_pl(char **values, t_holder *holder)
{
	t_plane		*plane;

	if (ft_memcmp(values[0], "pl", 2) == 0)
	{
		plane = (t_plane *)malloc(sizeof(*plane));
		plane->transform.position = parse_xyz(values[1]);
		plane->transform.orientation = parse_xyz(values[2]);
		plane->color = vec3_to_rgb(parse_xyz(values[3]));
		holder->scene.plane = plane;
		return (true);
	}
	return (false);
}

bool	read_sq(char **values, t_holder *holder)
{
	t_square	*square;

	if (ft_memcmp(values[0], "sq", 2) == 0)
	{
		square = (t_square *)malloc(sizeof(*square));
		square->transform.position = parse_xyz(values[1]);
		square->transform.orientation = parse_xyz(values[2]);
		square->side_size = ft_atoi(values[3]);
		square->color = vec3_to_rgb(parse_xyz(values[4]));
		holder->scene.square = square;
		return (true);
	}
	return (false);
}

bool	read_cy(char **values, t_holder *holder)
{
	t_cylinder	*cylinder;

	if (ft_memcmp(values[0], "cy", 2) == 0)
	{
		cylinder = (t_cylinder *)malloc(sizeof(*cylinder));
		cylinder->transform.position = parse_xyz(values[1]);
		cylinder->transform.orientation = parse_xyz(values[2]);
		cylinder->diameter = ft_atoi(values[3]);
		cylinder->height = ft_atoi(values[4]);
		cylinder->color = vec3_to_rgb(parse_xyz(values[5]));
		holder->scene.cylinder = cylinder;
		return (true);
	}
	return (false);
}

bool	read_tr(char **values, t_holder *holder)
{
	t_triangle	*triangle;

	if (ft_memcmp(values[0], "tr", 2) == 0)
	{
		triangle = (t_triangle *)malloc(sizeof(*triangle));
		triangle->p1 = parse_xyz(values[1]);
		triangle->p2 = parse_xyz(values[2]);
		triangle->p3 = parse_xyz(values[3]);
		triangle->color = vec3_to_rgb(parse_xyz(values[4]));
		holder->scene.triangle = triangle;
		return (true);
	}
	return (false);
}
