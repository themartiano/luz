/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:13:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 14:58:28 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

bool	read_sp(char **values, char *line, t_scene *scene)
{
	t_sphere	*sphere;
	t_object	*object;

	if (ft_memcmp(line, "sp ", 3) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		sphere = (t_sphere *)malloc(sizeof(*sphere));
		if (object == NULL || sphere == NULL)
			exit_error(scene, "MALLOC failed.");
		sphere->transform.position = parse_xyz(values[1]);
		sphere->radius = ft_atof(values[2]) / 2.0f;
		check_for_integer(scene, values[3]);
		sphere->color = vec3_to_rgb(parse_xyz(values[3]));
		if (!is_color_valid(sphere->color))
			exit_error(scene, "Sphere color out of range [0=>255].");
		object->object = sphere;
		object->type = 0;
		store_object(scene, object);
		return (true);
	}
	return (false);
}

bool	read_pl(char **values, char *line, t_scene *scene)
{
	t_plane		*plane;
	t_object	*object;

	if (ft_memcmp(line, "pl ", 3) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		plane = (t_plane *)malloc(sizeof(*plane));
		if (object == NULL || plane == NULL)
			exit_error(scene, "MALLOC failed.");
		plane->transform.position = parse_xyz(values[1]);
		plane->transform.orientation = parse_xyz(values[2]);
		if (!is_vec3_in_range(plane->transform.orientation, -1.0f, 1.0f))
			exit_error(scene, "Plane orientation out of range [-1.0=>1.0].");
		check_for_integer(scene, values[3]);
		plane->color = vec3_to_rgb(parse_xyz(values[3]));
		if (!is_color_valid(plane->color))
			exit_error(scene, "Plane color out of range [0=>255].");
		object->object = plane;
		object->type = 1;
		store_object(scene, object);
		return (true);
	}
	return (false);
}

bool	read_sq(char **values, char *line, t_scene *scene)
{
	t_square	*square;
	t_object	*object;

	if (ft_memcmp(line, "sq ", 3) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		square = (t_square *)malloc(sizeof(*square));
		if (object == NULL || square == NULL)
			exit_error(scene, "MALLOC failed.");
		square->transform.position = parse_xyz(values[1]);
		square->transform.orientation = parse_xyz(values[2]);
		if (!is_vec3_in_range(square->transform.orientation, -1.0f, 1.0f))
			exit_error(scene, "Square orientation out of range [-1.0=>1.0].");
		square->half_side_size = ft_atof(values[3]) / 2.0f;
		check_for_integer(scene, values[4]);
		square->color = vec3_to_rgb(parse_xyz(values[4]));
		if (!is_color_valid(square->color))
			exit_error(scene, "Square color out of range [0=>255].");
		object->object = square;
		object->type = 2;
		store_object(scene, object);
		return (true);
	}
	return (false);
}

bool	read_cy(char **values, char *line, t_scene *scene)
{
	t_cylinder	*cylinder;
	t_object	*object;

	if (ft_memcmp(line, "cy ", 3) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		cylinder = (t_cylinder *)malloc(sizeof(*cylinder));
		if (object == NULL || cylinder == NULL)
			exit_error(scene, "MALLOC failed.");
		cylinder->transform.position = parse_xyz(values[1]);
		cylinder->transform.orientation = parse_xyz(values[2]);
		if (!is_vec3_in_range(cylinder->transform.orientation, -1.0f, 1.0f))
			exit_error(scene, "Cylinder orientation out of range [-1.0=>1.0].");
		cylinder->radius = ft_atof(values[3]) / 2.0f;
		cylinder->height = ft_atof(values[4]);
		check_for_integer(scene, values[5]);
		cylinder->color = vec3_to_rgb(parse_xyz(values[5]));
		if (!is_color_valid(cylinder->color))
			exit_error(scene, "Cylinder color out of range [0=>255].");
		object->object = cylinder;
		object->type = 3;
		store_object(scene, object);
		return (true);
	}
	return (false);
}

bool	read_tr(char **values, char *line, t_scene *scene)
{
	t_triangle	*triangle;
	t_object	*object;

	if (ft_memcmp(line, "tr ", 3) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		triangle = (t_triangle *)malloc(sizeof(*triangle));
		if (object == NULL || triangle == NULL)
			exit_error(scene, "MALLOC failed.");
		triangle->p1 = parse_xyz(values[1]);
		triangle->p2 = parse_xyz(values[2]);
		triangle->p3 = parse_xyz(values[3]);
		check_for_integer(scene, values[4]);
		triangle->color = vec3_to_rgb(parse_xyz(values[4]));
		if (!is_color_valid(triangle->color))
			exit_error(scene, "Triangle color out of range [0=>255].");
		object->object = triangle;
		object->type = 4;
		store_object(scene, object);
		return (true);
	}
	return (false);
}
