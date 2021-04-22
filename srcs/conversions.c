/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conversions.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:06:23 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 15:46:49 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_vec3	parse_xyz(char *str)
{
	t_vec3	values;
	char	**input;

	input = ft_split(str, ',');
	values.x = ft_atoi(input[0]);
	values.y = ft_atoi(input[1]);
	values.z = ft_atoi(input[2]);
	return (values);
}

t_color	vec3_to_rgb(t_vec3 xyz)
{
	t_color	color;

	color.r = xyz.x;
	color.g = xyz.y;
	color.b = xyz.z;
	color.a = 0;
	return (color);
}

int	rgba_to_hex(t_color rgba)
{
	if (rgba.r > 255)
		rgba.r = 255;
	if (rgba.g > 255)
		rgba.g = 255;
	if (rgba.b > 255)
		rgba.b = 255;
	rgba.a = 0;
	return (rgba.a << 24 | rgba.r << 16 | rgba.g << 8 | rgba.b);
}

t_color	hex_to_rgba(int hex)
{
	t_color	rgba;

	rgba.r = hex & (0xFF << 16);
	rgba.g = hex & (0xFF << 8);
	rgba.b = hex & 0xFF;
	rgba.a = 0;
	if (rgba.r > 255)
		rgba.r = 255;
	if (rgba.g > 255)
		rgba.g = 255;
	if (rgba.b > 255)
		rgba.b = 255;
	return (rgba);
}
