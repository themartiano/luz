/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/11 15:25:59 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"
#include "readers.h"

static bool	read_r(char **values, char *line, t_scene *scene)
{
	int			screen_width;
	int			screen_height;
	static bool	resltn = false;

	if (ft_memcmp(line, "R ", 2) == 0)
	{
		if (resltn == true)
			exit_error(scene, "Multiple resolution entries.");
		scene->x_res = ft_atoi(values[1]);
		scene->y_res = ft_atoi(values[2]);
		mlx_get_screen_size(scene->mlx, &screen_width, &screen_height);
		if (scene->x_res > screen_width)
			scene->x_res = screen_width;
		if (scene->y_res > screen_height)
			scene->y_res = screen_height;
		if (scene->x_res <= 0 || scene->y_res <= 0)
			exit_error(scene, "Resolution must be greater than zero.");
		resltn = true;
		return (true);
	}
	return (false);
}

static bool	read_a(char **values, char *line, t_scene *scene)
{
	static bool	ambnt = false;

	if (ft_memcmp(line, "A ", 2) == 0)
	{
		if (ambnt == true)
			exit_error(scene, "Multiple ambient light entries.");
		scene->amb_light.brightness = ft_atof(values[1]);
		if (scene->amb_light.brightness < 0.0f
			|| scene->amb_light.brightness > 1.0f)
			exit_error(scene,
				"Ambient light brightness out of range [0.0=>1.0].");
		scene->amb_light.brightness /= 6.4f;
		scene->amb_light.color = vec3_to_rgb(parse_xyz(values[2]));
		if (!is_color_valid(scene->amb_light.color))
			exit_error(scene, "Ambient light color out of range [0=>255].");
		ambnt = true;
		return (true);
	}
	return (false);
}

static bool	read_c(char **values, char *line, t_scene *scene)
{
	t_camera	*camera;
	t_object	*object;
	float		theta;

	if (ft_memcmp(line, "c ", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		camera = (t_camera *)malloc(sizeof(*camera));
		if (object == NULL || camera == NULL)
			exit_error(scene, "MALLOC failed.");
		camera->transform.position = parse_xyz(values[1]);
		camera->transform.orientation = parse_xyz(values[2]);
		camera->transform.orientation.y *= -1.0f;
		if (!is_vec3_in_range(camera->transform.orientation, -1.0f, 1.0f))
			exit_error(scene, "Camera orientation out of range [-1.0=>1.0].");
		camera->fov = ft_atoi(values[3]);
		if (camera->fov < 0 || camera->fov > 180)
			exit_error(scene, "Camera FOV out of range [0=>180].");
		theta = camera->fov * M_PI / 180;
		camera->half_width = tan(theta / 2);
		camera->half_height = ((float)scene->y_res
				/ (float)scene->x_res) * camera->half_width;
		object->object = camera;
		object->type = 6;
		store_camera(scene, object);
		return (true);
	}
	return (false);
}

static bool	read_l(char **values, char *line, t_scene *scene)
{
	t_light		*light;
	t_object	*object;

	if (ft_memcmp(line, "l ", 2) == 0)
	{
		object = (t_object *)malloc(sizeof(*object));
		light = (t_light *)malloc(sizeof(*light));
		if (object == NULL || light == NULL)
			exit_error(scene, "MALLOC failed.");
		light->transform.position = parse_xyz(values[1]);
		light->brightness = ft_atof(values[2]);
		if (light->brightness < 0.0f || light->brightness > 1.0f)
			exit_error(scene, "Light brightness out of range [0.0=>1.0].");
		light->color = vec3_to_rgb(parse_xyz(values[3]));
		if (!is_color_valid(light->color))
			exit_error(scene, "Light color out of range [0=>255].");
		object->object = light;
		object->type = 5;
		store_light(scene, object);
		return (true);
	}
	return (false);
}

void	read_scene(int fd, t_scene *scene)
{
	char	*line;
	char	**values;
	int		rv;
	int		i;

	rv = 1;
	line = NULL;
	values = NULL;
	while (rv == 1)
	{
		rv = get_next_line(fd, &line);
		values = ft_split(line, ' ');
		verify_values(scene, values, 1);
		if (read_r(values, line, scene) || read_a(values, line, scene)
			|| read_c(values, line, scene) || read_l(values, line, scene)
			|| read_sp(values, line, scene) || read_pl(values, line, scene)
			|| read_sq(values, line, scene) || read_cy(values, line, scene)
			|| read_tr(values, line, scene))
			i = 0;
		free(line);
		i = 0;
		while (values != NULL && values[i])
			free(values[i++]);
		free(values);
	}
}
