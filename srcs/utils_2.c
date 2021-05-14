/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/12 15:50:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 14:54:17 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	set_hit_color(t_scene *scene, t_hit_record *hit_rec)
{
	if (hit_rec->color.r != 0 && hit_rec->color.g != 0 && hit_rec->color.b != 0)
	{
		hit_rec->color = divide_color(sum_colors(hit_rec->color,
					hit_rec->hit_color), 2);
	}
	else
		hit_rec->color = hit_rec->hit_color;
	hit_rec->color = set_color(
			((float)hit_rec->color.r) + ((float)scene->amb_light.color.r
				* scene->amb_light.brightness),
			((float)hit_rec->color.g) + ((float)scene->amb_light.color.g
				* scene->amb_light.brightness),
			((float)hit_rec->color.b) + ((float)scene->amb_light.color.b
				* scene->amb_light.brightness));
	calc_lights(scene, hit_rec);
}

void	verify_values(t_scene *scene, char **input, int start)
{
	int	i;
	int	j;

	i = -1;
	while (++i < start)
	{
		if (!input[i])
			break ;
	}
	while (input[i])
	{
		j = 0;
		while (input[i][j])
		{
			if (!ft_isdigit(input[i][j]) && !ft_charinset(input[i][j], ".-,"))
				exit_error(scene, "Invalid characters detected.");
			j++;
		}
		i++;
	}
}

void	check_for_integer(t_scene *scene, char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!ft_isdigit(str[i]) && !ft_charinset(str[i], ",-"))
			exit_error(scene, "Invalid characters detected.");
		i++;
	}
}

void	check_resolution(t_scene *scene)
{
	if (scene->x_res <= 0 || scene->y_res <= 0)
	{
		exit_error(scene, "Resolution not set.");
	}
}
