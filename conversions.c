/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conversions.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:06:23 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/13 13:06:25 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_xyz	parse_xyz(char *str)
{
	t_xyz	values;
	char	**input;

	input = ft_split(str, ',');
	values.x = ft_atoi(input[0]);
	values.y = ft_atoi(input[1]);
	values.z = ft_atoi(input[2]);
	return (values);
}

t_color	xyz_to_rgb(t_xyz xyz)
{
	t_color	color;

	color.r = xyz.x;
	color.g = xyz.y;
	color.b = xyz.z;
	color.a = 0;
	return (color);
}
