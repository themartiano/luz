/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/12 15:50:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/12 16:01:22 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	set_hit_color(t_scene *scene, t_hit_record *hit_rec, t_color color)
{
	if (hit_rec->color.r != 0 && hit_rec->color.g != 0 && hit_rec->color.b != 0)
		hit_rec->color = divide_color(sum_colors(hit_rec->color, color), 2);
	else
		hit_rec->color = color;
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
