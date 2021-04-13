/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/10 14:51:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 15:22:12 by ejuliao-         ###   ########.fr       */
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

int	exit_error(char *message)
{
	ft_putstr("Error\n");
	ft_putstr(message);
	return (1);
}

void	put_pixel(t_img *img_data, int x, int y, int color)
{
	char	*dst;

	dst = img_data->addr + (y * img_data->line_length + x
			* (img_data->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

void	fill_image(t_img *img_data, int width, int height, int color)
{
	int	x;
	int	y;

	y = 0;
	while (y < height)
	{
		x = 0;
		while (x < width)
		{
			put_pixel(img_data, x, y, color);
			x++;
		}
		y++;
	}
}
